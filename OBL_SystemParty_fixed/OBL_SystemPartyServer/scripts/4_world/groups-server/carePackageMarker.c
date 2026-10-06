// ============================================================
// Мітки для сторонніх івент-контейнерів (Care Packages V2 і т.п.)
//
// Працює без залежності від скриптів чужого мода:
// ловимо спавн предмета і порівнюємо його класнейм зі списком
// з конфігу $profile:OBLParty/CarePackageMarkers.json
//
// Конфіг створюється автоматично при першому старті сервера.
// ============================================================

class OBLEventMarkerRule {
	// Частина класнейму (без регістру). Напр. "CarePackage" зловить
	// CarePackage_Container_Green, CarePackagesContainer і т.д.
	string classnameContains = "";
	// Підпис мітки на карті
	string markerName = "Гуманітарка";
	// Іконка (шлях у PBO)
	string icon = "OBL_SystemParty\\gui\\icons\\ping.paa";
	int colorR = 255;
	int colorG = 255;
	int colorB = 255;
	// 1 = мітку видно (карта + 3D), 0 = правило вимкнене
	int enabled = 1;
	// Затримка в секундах перед появою мітки (щоб контейнер встиг впасти).
	// 0 = мітка одразу при спавні.
	int delaySeconds = 0;

	void Init(string contains_, string name_, string icon_, int r, int g, int b, int delay_) {
		classnameContains = contains_;
		markerName = name_;
		icon = icon_;
		colorR = r;
		colorG = g;
		colorB = b;
		enabled = 1;
		delaySeconds = delay_;
	}
}

class OBLEventMarkerConfig {

	int enableEventMarkers = 1;
	// 1 = писати в лог класнейми створених міток (для налаштування)
	int debugLog = 1;
	ref array<ref OBLEventMarkerRule> rules = new array<ref OBLEventMarkerRule>();

	static ref OBLEventMarkerConfig g_OBLEventMarkerConfig;

	// OBL FIX: EEInit runs for every spawned item; cache rule index per classname (-1 = no rule)
	[NonSerialized()]
	ref map<string, int> m_RuleCache = new map<string, int>();

	static string GetPath() {
		return OBLPartyConstants.SAVE_PREFIX + "CarePackageMarkers.json";
	}

	static OBLEventMarkerConfig Get() {
		if (!g_OBLEventMarkerConfig)
			g_OBLEventMarkerConfig = Load();
		return g_OBLEventMarkerConfig;
	}

	static void Delete() {
		if (g_OBLEventMarkerConfig)
			delete g_OBLEventMarkerConfig;
	}

	static OBLEventMarkerConfig Load() {
		OBLEventMarkerConfig cfg;
		if (!FileExist(GetPath())) {
			cfg = LoadDefault();
			JsonFileLoader<OBLEventMarkerConfig>.JsonSaveFile(GetPath(), cfg);
			return cfg;
		}
		cfg = new OBLEventMarkerConfig();
		JsonFileLoader<OBLEventMarkerConfig>.JsonLoadFile(GetPath(), cfg);
		if (!cfg.rules)
			cfg.rules = new array<ref OBLEventMarkerRule>();
		return cfg;
	}

	static OBLEventMarkerConfig LoadDefault() {
		OBLEventMarkerConfig cfg = new OBLEventMarkerConfig();

		// Care Packages V2 (Steam 2691041685) — контейнери
		OBLEventMarkerRule carePackage = new OBLEventMarkerRule();
		carePackage.Init("CarePackage", "Гуманітарка", "OBL_SystemParty\\gui\\icons\\ping.paa", 120, 255, 120, 20);
		cfg.rules.Insert(carePackage);

		// Care Packages V2 — альтернативний префікс класів контейнерів
		OBLEventMarkerRule cpAlt = new OBLEventMarkerRule();
		cpAlt.Init("CarePackages", "Гуманітарка", "OBL_SystemParty\\gui\\icons\\ping.paa", 120, 255, 120, 20);
		cfg.rules.Insert(cpAlt);

		return cfg;
	}

	// Повертає правило, яке підходить під класнейм, або null
	OBLEventMarkerRule FindRule(string classname) {
		if (enableEventMarkers != 1 || !rules || rules.Count() == 0)
			return null;

		if (!m_RuleCache)
			m_RuleCache = new map<string, int>();
		int cached;
		if (m_RuleCache.Find(classname, cached)) {
			if (cached < 0)
				return null;
			return rules.Get(cached);
		}

		string lower = classname;
		lower.ToLower();

		for (int i = 0; i < rules.Count(); i++) {
			OBLEventMarkerRule rule = rules.Get(i);
			if (!rule || rule.enabled != 1 || rule.classnameContains == "")
				continue;
			string needle = rule.classnameContains;
			needle.ToLower();
			if (lower.Contains(needle)) {
				m_RuleCache.Set(classname, i);
				return rule;
			}
		}
		m_RuleCache.Set(classname, -1);
		return null;
	}
}

// ------------------------------------------------------------
// Хук на спавн предметів (контейнери Care Packages — ItemBase)
// ------------------------------------------------------------
modded class ItemBase {

	int m_OBLEventMarkerUID = 0;
	ref OBLEventMarkerRule m_OBLEventMarkerRule = null;

	override void EEInit() {
		super.EEInit();

		if (!GetGame() || !GetGame().IsServer())
			return;

		OBLEventMarkerRule rule = OBLEventMarkerConfig.Get().FindRule(GetType());
		if (!rule)
			return;

		m_OBLEventMarkerRule = rule;

		if (rule.delaySeconds > 0)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OBL_CreateEventMarker, rule.delaySeconds * 1000, false);
		else
			OBL_CreateEventMarker();
	}

	void OBL_CreateEventMarker() {
		if (m_OBLEventMarkerUID != 0 || !m_OBLEventMarkerRule)
			return;

		OBLStaticMarkerManager mgr = OBLStaticMarkerManager.Get();
		if (!mgr)
			return;

		int color = ARGB(255, m_OBLEventMarkerRule.colorR, m_OBLEventMarkerRule.colorG, m_OBLEventMarkerRule.colorB);
		OBLServerMarker marker = mgr.AddTempServerMarker(m_OBLEventMarkerRule.markerName, GetPosition(), m_OBLEventMarkerRule.icon, color);
		if (marker) {
			m_OBLEventMarkerUID = marker.uid;
			if (OBLEventMarkerConfig.Get().debugLog == 1)
				OBLLogger.Debug("[EventMarker] " + GetType() + " -> \"" + m_OBLEventMarkerRule.markerName + "\" UID=" + m_OBLEventMarkerUID + " pos=" + GetPosition());
		}
	}

	void OBL_RemoveEventMarker() {
		if (m_OBLEventMarkerUID == 0)
			return;

		OBLStaticMarkerManager mgr = OBLStaticMarkerManager.Get();
		if (mgr) {
			OBLServerMarker marker = mgr.FindTempMarker(m_OBLEventMarkerUID);
			if (marker)
				mgr.RemoveServerMarker(marker);
		}
		m_OBLEventMarkerUID = 0;
	}

	override void EEDelete(EntityAI parent) {
		super.EEDelete(parent);

		if (!GetGame() || !GetGame().IsServer())
			return;

		if (m_OBLEventMarkerRule)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OBL_CreateEventMarker);

		OBL_RemoveEventMarker();
		m_OBLEventMarkerRule = null;
	}
}
