// ===================================================================
//  Server-side "Магазин" (warehouse claim) manager.
//
//  Flow:
//    client SHOP_LIST_REQUEST  -> GET api_warehouse.php?action=list  -> SHOP_LIST_SYNC
//    client SHOP_CLAIM_REQUEST -> GET api_warehouse.php?action=claim -> SHOP_CLAIM_RESULT
//
//  This manager NEVER spawns items. Claiming just sets is_delivered=0 on the
//  site, and the existing MyWebStore delivery mod spawns the item.
//
//  Config (OBLPartyMainConfig.json on the server):
//    shopApiUrl   e.g. "https://oblivion-store.vn.ua"
//    shopServerId e.g. 1
//    shopSecretKey "..."   (from `servers` table)
// ===================================================================

// ---- JSON-mapped response classes ----
class OBLShopApiItem {
	int    purchase_id;
	string item_name;
	string classname;
	int    qty;
}

class OBLShopListResponse {
	string                       status;
	int                          count;
	ref array<ref OBLShopApiItem> items;
}

class OBLShopClaimResponse {
	string status;
	int    purchase_id;
	string item_name;
	string msg;
}

// ---- REST callbacks ----
class OBLShopListCallback : RestCallback {
	string ownerSteamId;   // who asked

	void OBLShopListCallback(string steamId) {
		ownerSteamId = steamId;
	}

	override void OnSuccess(string data, int dataSize) {
		OBLShopManager mgr = OBLShopManager.Get();
		if (mgr)
			mgr.OnListResponse(ownerSteamId, data);
	}

	override void OnError(int errorCode) {
		OBLShopManager mgr = OBLShopManager.Get();
		if (mgr)
			mgr.OnListError(ownerSteamId, errorCode);
	}
}

class OBLShopClaimCallback : RestCallback {
	string ownerSteamId;

	void OBLShopClaimCallback(string steamId) {
		ownerSteamId = steamId;
	}

	override void OnSuccess(string data, int dataSize) {
		OBLShopManager mgr = OBLShopManager.Get();
		if (mgr)
			mgr.OnClaimResponse(ownerSteamId, data);
	}

	override void OnError(int errorCode) {
		OBLShopManager mgr = OBLShopManager.Get();
		if (mgr)
			mgr.OnClaimError(ownerSteamId, errorCode);
	}
}

// ---- Manager ----
class OBLShopManager {

	static ref OBLShopManager g_instance;

	// keep callbacks alive while the HTTP request is in flight
	ref array<ref RestCallback> pendingCallbacks = new array<ref RestCallback>();

	// OBL FIX: claim, що очікує відправки — окремо для кожного гравця
	// (раніше був один слот на весь сервер, і одночасні claim'и різних гравців перетирали один одного)
	ref map<string, int> m_pendingClaims = new map<string, int>();
	ref map<string, int> m_claimRetries  = new map<string, int>();
	ref map<string, int> m_claimStarted  = new map<string, int>();
	const int CLAIM_STUCK_MS = 30000;
	// антиспам: час останнього запиту гравця (мс), окремо для списку і видачі
	ref map<string, int> m_lastRequestTime = new map<string, int>();
	// гравці, яким уже заплановано відкладений запит списку
	ref TStringSet m_deferredList = new TStringSet();
	const int REQUEST_COOLDOWN_MS = 1500;
	const int MAX_KEPT_CALLBACKS  = 256;

	void OBLShopManager() {
		GetDayZGame().Event_OnRPC.Insert(OnRPC_Shop);
	}

	void ~OBLShopManager() {
		GetDayZGame().Event_OnRPC.Remove(OnRPC_Shop);
	}

	static OBLShopManager Get() {
		if (!g_instance)
			g_instance = new OBLShopManager();
		return g_instance;
	}

	bool ShopEnabled() {
		return OBLPartyMainConfig.Get().enableShop;
	}

	string ApiUrl() {
		string u = OBLPartyMainConfig.Get().shopApiUrl;
		u.Replace("\\", "/");
		// strip trailing slash
		while (u.Length() > 0 && u.IndexOf("/") == u.Length() - 1)
			u = u.Substring(0, u.Length() - 1);
		return u;
	}

	// повертає 0, якщо запит можна виконати зараз, інакше — скільки мс ще чекати
	int CooldownLeft(string key) {
		int now = GetGame().GetTime();
		if (m_lastRequestTime.Contains(key)) {
			int left = REQUEST_COOLDOWN_MS - (now - m_lastRequestTime.Get(key));
			if (left > 0)
				return left;
		}
		m_lastRequestTime.Set(key, now);
		return 0;
	}

	void DeferredListRequest(string steamId) {
		m_deferredList.Remove(steamId);
		PlayerBase pb = GetPlayerBySteamId(steamId);
		if (!pb || !pb.GetIdentity())
			return;
		m_lastRequestTime.Set("L" + steamId, GetGame().GetTime());
		HandleListRequest(pb.GetIdentity());
	}

	void KeepCallback(RestCallback cb) {
		pendingCallbacks.Insert(cb);
		// OBL FIX: список ріс без обмежень; найстаріші запити давно завершені
		while (pendingCallbacks.Count() > MAX_KEPT_CALLBACKS)
			pendingCallbacks.RemoveOrdered(0);
	}

	// ---- RPC entry ----
	void OnRPC_Shop(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
		if (!sender)
			return;
		if (rpc_type != OBLPartyRPCs.SHOP_LIST_REQUEST && rpc_type != OBLPartyRPCs.SHOP_CLAIM_REQUEST)
			return;
		string steamId = sender.GetPlainId();

		if (rpc_type == OBLPartyRPCs.SHOP_LIST_REQUEST) {
			int listWait = CooldownLeft("L" + steamId);
			if (listWait > 0) {
				// OBL FIX: замість відмови — виконуємо один відкладений запит
				if (!m_deferredList.Contains(steamId)) {
					m_deferredList.Insert(steamId);
					GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DeferredListRequest, listWait, false, steamId);
				}
				return;
			}
			HandleListRequest(sender);
		} else if (rpc_type == OBLPartyRPCs.SHOP_CLAIM_REQUEST) {
			int purchaseId;
			if (!ctx.Read(purchaseId))
				return;
			bool claimRunning = m_pendingClaims.Contains(steamId) && m_claimStarted.Contains(steamId) && GetGame().GetTime() - m_claimStarted.Get(steamId) < CLAIM_STUCK_MS;
			if (claimRunning || CooldownLeft("C" + steamId) > 0) {
				SendClaimResult(steamId, false, "Зачекайте, попередній запит ще обробляється.");
				return;
			}
			HandleClaimRequest(sender, purchaseId);
		}
	}

	// ---- LIST ----
	void HandleListRequest(PlayerIdentity sender) {
		if (!ShopEnabled()) {
			SendClaimResult(sender.GetPlainId(), false, "Магазин вимкнено на цьому сервері.");
			return;
		}
		string steamId  = sender.GetPlainId();
		string serverIp = GetServerIp();

		OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
		string path = "/api_warehouse.php?action=list&key=" + cfg.shopSecretKey + "&server_id=" + cfg.shopServerId.ToString() + "&sid=" + steamId + "&server_ip=" + serverIp;

		RestContext ctx = GetRestApi().GetRestContext(ApiUrl());
		if (!ctx) {
			SendClaimResult(steamId, false, "Не вдалося звʼязатися з магазином.");
			return;
		}
		OBLShopListCallback cb = new OBLShopListCallback(steamId);
		KeepCallback(cb);
		ctx.GET(cb, path);
		// OBL FIX: не пишемо в лог URL з секретним ключем
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("[Shop] LIST request for " + steamId + " -> " + ApiUrl());
	}

	void OnListResponse(string steamId, string data) {
		OBLShopListResponse resp = new OBLShopListResponse();
		JsonSerializer js = new JsonSerializer();
		string err;
		bool ok = js.ReadFromString(resp, data, err);

		if (!ok) {
			OBLLogger.Warn("[Shop] LIST parse failed: " + err + " raw=" + data);
			// показуємо сире повідомлення з сайту, якщо є коротке
			string shortRaw = data;
			if (shortRaw.Length() > 80)
				shortRaw = shortRaw.Substring(0, 80);
			SendClaimResult(steamId, false, "Помилка читання складу: " + shortRaw);
			SendListSync(steamId, null);
			return;
		}

		if (resp.status != "success") {
			OBLLogger.Warn("[Shop] LIST status!=success raw=" + data);
			string sr = data;
			if (sr.Length() > 90)
				sr = sr.Substring(0, 90);
			SendClaimResult(steamId, false, "Магазин: " + sr);
			SendListSync(steamId, null);
			return;
		}

		// успіх: навіть якщо порожньо — просто шлемо порожній список
		SendListSync(steamId, resp.items);
		if (!resp.items || resp.items.Count() == 0)
			SendClaimResult(steamId, false, "Склад порожній.");
	}

	void OnListError(string steamId, int errorCode) {
		OBLLogger.Warn("[Shop] LIST http error " + errorCode + " for " + steamId);
		SendClaimResult(steamId, false, "Магазин недоступний (код " + errorCode + ").");
		SendListSync(steamId, null);
	}

	// ---- CLAIM ----
	void HandleClaimRequest(PlayerIdentity sender, int purchaseId) {
		if (!ShopEnabled()) {
			SendClaimResult(sender.GetPlainId(), false, "Магазин вимкнено на цьому сервері.");
			return;
		}
		string claimSteam = sender.GetPlainId();
		m_pendingClaims.Set(claimSteam, purchaseId);
		m_claimRetries.Set(claimSteam, 0);
		m_claimStarted.Set(claimSteam, GetGame().GetTime());
		// невелика затримка дає RestApi звільнити контекст після попереднього GET
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DoClaimRequest, 150, false, claimSteam);
	}

	void DoClaimRequest(string steamId) {
		if (steamId == "" || !m_pendingClaims.Contains(steamId))
			return;
		int purchaseId = m_pendingClaims.Get(steamId);
		string serverIp = GetServerIp();

		OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
		string path = "/api_warehouse.php?action=claim&key=" + cfg.shopSecretKey + "&server_id=" + cfg.shopServerId.ToString() + "&sid=" + steamId + "&server_ip=" + serverIp + "&purchase_id=" + purchaseId.ToString();

		RestContext ctx = GetRestApi().GetRestContext(ApiUrl());
		if (!ctx) {
			m_pendingClaims.Remove(steamId);
			SendClaimResult(steamId, false, "Не вдалося звʼязатися з магазином.");
			return;
		}
		OBLShopClaimCallback cb = new OBLShopClaimCallback(steamId);
		KeepCallback(cb);
		ctx.GET(cb, path);
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("[Shop] CLAIM request for " + steamId + " purchase " + purchaseId);
	}

	void OnClaimResponse(string steamId, string data) {
		OBLShopClaimResponse resp = new OBLShopClaimResponse();
		JsonSerializer js = new JsonSerializer();
		string err;
		if (!js.ReadFromString(resp, data, err)) {
			m_pendingClaims.Remove(steamId);
			m_claimRetries.Remove(steamId);
			OBLLogger.Warn("[Shop] CLAIM parse failed: " + err + " raw=" + data);
			SendClaimResult(steamId, false, "Помилка читання відповіді складу.");
			return;
		}
		m_pendingClaims.Remove(steamId);
		m_claimRetries.Remove(steamId);
		if (resp.status == "success") {
			SendClaimResult(steamId, true, "Предмет [" + resp.item_name + "] видається у гру. Зачекайте кілька секунд.");
		} else {
			string m = resp.msg;
			if (m == "")
				m = "Не вдалося забрати предмет.";
			SendClaimResult(steamId, false, m);
		}
	}

	void OnClaimError(string steamId, int errorCode) {
		OBLLogger.Warn("[Shop] CLAIM http error " + errorCode + " for " + steamId);
		// код 7 = тимчасовий збій зʼєднання/таймаут REST. Пробуємо ще раз (1 раз).
		int retries = 0;
		if (m_claimRetries.Contains(steamId))
			retries = m_claimRetries.Get(steamId);
		if (errorCode == 7 && retries < 1 && m_pendingClaims.Contains(steamId)) {
			m_claimRetries.Set(steamId, retries + 1);
			SendClaimResult(steamId, false, "Повторна спроба видачі...");
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DoClaimRequest, 400, false, steamId);
			return;
		}
		m_pendingClaims.Remove(steamId);
		m_claimRetries.Remove(steamId);
		SendClaimResult(steamId, false, "Магазин недоступний (код " + errorCode + ").");
	}

	// ---- Replies to client ----
	void SendListSync(string steamId, array<ref OBLShopApiItem> items) {
		PlayerBase pb = GetPlayerBySteamId(steamId);
		if (!pb || !pb.GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		int n = 0;
		if (items)
			n = items.Count();
		rpc.Write(n);
		for (int i = 0; i < n; i++) {
			OBLShopApiItem it = items.Get(i);
			rpc.Write(it.purchase_id);
			rpc.Write(it.item_name);
			rpc.Write(it.classname);
			int q = it.qty;
			if (q < 1) q = 1;
			rpc.Write(q);
		}
		rpc.Send(pb, OBLPartyRPCs.SHOP_LIST_SYNC, true, pb.GetIdentity());
	}

	void SendClaimResult(string steamId, bool ok, string message) {
		PlayerBase pb = GetPlayerBySteamId(steamId);
		if (!pb || !pb.GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(ok);
		rpc.Write(message);
		rpc.Send(pb, OBLPartyRPCs.SHOP_CLAIM_RESULT, true, pb.GetIdentity());
	}

	// ---- Helpers ----
	PlayerBase GetPlayerBySteamId(string steamId) {
		array<Man> players = new array<Man>();
		GetGame().GetWorld().GetPlayerList(players);
		foreach (Man m : players) {
			PlayerBase pb = PlayerBase.Cast(m);
			if (pb && pb.GetIdentity() && pb.GetIdentity().GetPlainId() == steamId)
				return pb;
		}
		return null;
	}

	string GetServerIp() {
		// best-effort; site only logs it, auth is by key+server_id
		string ip = "";
		GetGame().CommandlineGetParam("ip", ip);
		if (ip == "")
			ip = "0.0.0.0";
		return ip;
	}
}
