// One item sitting on the warehouse (web-store), waiting to be claimed in-game.
class OBLShopItem {
	int    purchaseId;
	string itemName;
	string classname;
	int    qty;

	void OBLShopItem(int pid, string name, string cls, int q) {
		purchaseId = pid;
		itemName   = name;
		classname  = cls;
		qty        = q;
	}
}

// Client-side holder + sender for the "Магазин" (warehouse claim) tab.
// Talks to the server via the global GROUP_RPC channel (no group needed).
class OBLShopClient {

	static ref OBLShopClient g_instance;

	ref array<ref OBLShopItem> items = new array<ref OBLShopItem>();

	// status flags the page reads
	bool   loading      = false;
	bool   lastClaimOk  = false;
	bool   gotResponse  = false; // чи прийшла хоч одна відповідь складу
	string lastMessage  = "";
	int    lastClaimId  = 0;
	int    loadingSince = 0;
	const int LOADING_TIMEOUT_MS = 15000;

	static OBLShopClient Get() {
		if (!g_instance)
			g_instance = new OBLShopClient();
		return g_instance;
	}

	// ---- Outgoing (client -> server) ----

	void RequestList() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb)
			return;
		loading = true;
		loadingSince = GetGame().GetTime();
		// Sent on a dedicated RPC channel (not wrapped in GROUP_RPC) so the
		// warehouse works even for players who are not in a group. The server's
		// OBLShopManager hooks Event_OnRPC and matches rpc_type directly.
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(pb, OBLPartyRPCs.SHOP_LIST_REQUEST, true);
	}

	void RequestClaim(int purchaseId) {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || purchaseId <= 0)
			return;
		lastClaimId = purchaseId;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(purchaseId);
		rpc.Send(pb, OBLPartyRPCs.SHOP_CLAIM_REQUEST, true);
	}

	// якщо сервер не відповів — не показуємо «Завантаження...» вічно
	void CheckLoadingTimeout() {
		if (loading && GetGame().GetTime() - loadingSince > LOADING_TIMEOUT_MS) {
			loading = false;
			lastMessage = "Магазин не відповідає. Спробуйте оновити ще раз.";
		}
	}

	// ---- Incoming (server -> client) ----

	void OnListSync(ParamsReadContext ctx) {
		loading = false;
		gotResponse = true;
		// успішна відповідь списку прибирає попереднє повідомлення про помилку
		lastMessage = "";
		items.Clear();
		int count = 0;
		if (!ctx.Read(count))
			return;
		for (int i = 0; i < count; i++) {
			int    pid;
			string name;
			string cls;
			int    q;
			if (!ctx.Read(pid))  return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(cls))  return;
			if (!ctx.Read(q))    return;
			items.Insert(new OBLShopItem(pid, name, cls, q));
		}
	}

	void OnClaimResult(ParamsReadContext ctx) {
		bool   ok;
		string msg;
		if (!ctx.Read(ok))  return;
		if (!ctx.Read(msg)) return;
		lastClaimOk = ok;
		lastMessage = msg;
		// склад не перезавантажуємо автоматично: просто прибираємо виданий предмет зі списку
		if (ok)
			RemoveItem(lastClaimId);
	}

	void RemoveItem(int purchaseId) {
		for (int i = items.Count() - 1; i >= 0; i--) {
			OBLShopItem it = items.Get(i);
			if (it && it.purchaseId == purchaseId)
				items.Remove(i);
		}
	}
}
