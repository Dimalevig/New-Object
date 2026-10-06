class OBLMarkerVisibilityManager {

	static int CURRENT_VERSION = 3;
	int version = 0;
	
	ref array<ref OBLMarkerVisibilityEntry> entries = new array<ref OBLMarkerVisibilityEntry>();
	ref array<ref OBLGlobalVisibilityEntry> globalentries = new array<ref OBLGlobalVisibilityEntry>();
	
	int globalVisiblityState;
	string pingMarkerIcon = "OBL_SystemParty\\gui\\icons\\ping.paa";
	bool compassEnabled = true;
	bool playerlistEnabled = true;
	bool disableShowClantextures = false;
	int chatSize = 15;
	int teammate3DMarkerDistance = -1; // -1 = server max, 0 = off, >0 = meters
	
	static ref OBLMarkerVisibilityManager g_OBLMarkerVisibilityManager;
	
	static void Delete() {
		if (g_OBLMarkerVisibilityManager)
			delete g_OBLMarkerVisibilityManager;
	}
	
	void ~OBLMarkerVisibilityManager() {
		Save();
	}
	
	static OBLMarkerVisibilityManager Get() {
		if (!g_OBLMarkerVisibilityManager) {
			g_OBLMarkerVisibilityManager = Load();
		}
		return g_OBLMarkerVisibilityManager;
	}
	
	static OBLMarkerVisibilityManager Load() {
		OBLMarkerVisibilityManager mgr;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES)) {
			mgr = LoadDefault();
			JsonFileLoader<OBLMarkerVisibilityManager>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES, mgr);
			return mgr;
		}
		mgr = new OBLMarkerVisibilityManager();
		OBLLogger.Debug("JsonLoadFile OBLMarkerVisibilityManager.json");
		JsonFileLoader<OBLMarkerVisibilityManager>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES, mgr);
		if (mgr.version != CURRENT_VERSION) {
			OBLLogger.Debug("Upgrading Visibility Manager Version from " + mgr.version + " to " + CURRENT_VERSION);
			mgr.UpgradeVersion(mgr.version, CURRENT_VERSION);
			mgr.Save();
		}
		return mgr;
	}
	
	static OBLMarkerVisibilityManager LoadDefault() {
		OBLMarkerVisibilityManager def = new OBLMarkerVisibilityManager;
		def.version = CURRENT_VERSION;
		def.globalVisiblityState = 0;
		def.pingMarkerIcon = "OBL_SystemParty\\gui\\icons\\ping.paa";
		def.compassEnabled = true;
		def.playerlistEnabled = true;
		def.teammate3DMarkerDistance = -1;
		return def;
	}
	
	void UpgradeVersion(int from, int to) {
		version = CURRENT_VERSION;
		if (from < 1) {
			pingMarkerIcon = "OBL_SystemParty\\gui\\icons\\ping.paa";
			compassEnabled = true;
			playerlistEnabled = true;
		}
		if (from < 2) {
			chatSize = 15;
		}
		if (from < 3) {
			teammate3DMarkerDistance = -1;
		}
	}
	
	int GetChatSize() {
		return Math.Max(7, chatSize);
	}

	int GetTeammate3DMarkerDistance() {
		return teammate3DMarkerDistance;
	}

	void SetTeammate3DMarkerDistance(int distance) {
		teammate3DMarkerDistance = distance;
		Save();
	}
	
	void Save() {
		for (int i = 0; i < entries.Count(); i++) {
			OBLMarkerVisibilityEntry entry = entries.Get(i);
			if (entry.displaystate == 0) {
				entries.Remove(i--);
			}
		}
		JsonFileLoader<OBLMarkerVisibilityManager>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES, this);
	}
	
	string GetPingMarkerIcon() {
		if (!FileExist(pingMarkerIcon) || pingMarkerIcon.Length() < 4)
			return "OBL_SystemParty\\gui\\icons\\ping.paa";
		return pingMarkerIcon;
	}
	
	void ResetPingToDefault() {
		SetPingMarkerIcon("OBL_SystemParty\\gui\\icons\\ping.paa");
		chatSize = 15;
	}
	
	void ResetPingToLast() {
		OBLMarkerVisibilityManager mgr = new OBLMarkerVisibilityManager();
		if (FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES)) {
			JsonFileLoader<OBLMarkerVisibilityManager>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER_STATES, mgr);
			SetPingMarkerIcon(mgr.pingMarkerIcon);
			chatSize = mgr.chatSize;
		} else {
			ResetPingToDefault();
		}
	}
	
	void SetPingMarkerIcon(string icon) {
		this.pingMarkerIcon = icon;
	}
	
	TStringArray GetPingMarkerIcons() {
		TStringArray arr = new TStringArray();
		foreach (string icon : OBLPartyMainConfig.Get().availableIcons) {
			arr.Insert(icon);
		}
		if (arr.Find("OBL_SystemParty\\gui\\icons\\ping.paa") == -1 && arr.Find("OBL_SystemParty/gui/icons/ping.paa") == -1)
			arr.Insert("OBL_SystemParty\\gui\\icons\\ping.paa");
		return arr;
	}
	
	OBLMarkerVisibilityEntry GetVisibilityOrAdd(int uid) {
		foreach (OBLMarkerVisibilityEntry entry : entries) {
			if (entry.uid == uid)
				return entry;
		}
		OBLMarkerVisibilityEntry ent = new OBLMarkerVisibilityEntry();
		ent.uid = uid;
		ent.displaystate = 0;
		entries.Insert(ent);
		return ent;
	}
	
	OBLGlobalVisibilityEntry GetGlobalVisibilityOrAdd(OBLMarkerType type) {
		foreach (OBLGlobalVisibilityEntry entry : globalentries) {
			if (entry.type == type)
				return entry;
		}
		OBLGlobalVisibilityEntry ent = new OBLGlobalVisibilityEntry();
		ent.type = type;
		ent.displaystate = 0;
		globalentries.Insert(ent);
		return ent;
	}
	
	bool Is3DVisiblie(int uid, OBLMarkerType type, bool testType = true) {
		if (GetVisibilityOrAdd(uid).displaystate != 0)
			return false;
		return !testType || IsGlobal3DVisible(type);
	}
	
	bool IsMapVisible(int uid, OBLMarkerType type, bool testType = true) {
		if (GetVisibilityOrAdd(uid).displaystate == 2)
			return false;
		return !testType || IsGlobal2DVisible(type);
	}
	
	bool IsGlobal3DVisible(OBLMarkerType type) {
		return (GetGlobalVisibilityOrAdd(type).displaystate == 0);
	}
	
	bool IsGlobal2DVisible(OBLMarkerType type) {
		return (GetGlobalVisibilityOrAdd(type).displaystate != 2);
	}
	
	int GetNextState() {
		globalVisiblityState++;
		globalVisiblityState = globalVisiblityState % 6;
		if (globalVisiblityState == 0) {
			SetAllGlobalStates(0);
		} else if (globalVisiblityState == 1) {
			SetAllGlobalStates(0);
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_STATIC).displaystate = 1;
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_DYNAMIC).displaystate = 1;
		} else if (globalVisiblityState == 2) {
			SetAllGlobalStates(0);
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_STATIC).displaystate = 1;
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_DYNAMIC).displaystate = 1;
			GetGlobalVisibilityOrAdd(OBLMarkerType.PRIVATE_MARKER).displaystate = 1;
		} else if (globalVisiblityState == 3) {
			SetAllGlobalStates(1);
			GetGlobalVisibilityOrAdd(OBLMarkerType.GROUP_PLAYER_MARKER).displaystate = 0;
			GetGlobalVisibilityOrAdd(OBLMarkerType.GROUP_PING).displaystate = 0;
		} else if (globalVisiblityState == 4) {
			SetAllGlobalStates(1);
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_STATIC).displaystate = 0;
			GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_DYNAMIC).displaystate = 0;
		} else {
			SetAllGlobalStates(1);
		}
		Save();
		return globalVisiblityState;
	}
	
	void SetAllGlobalStates(int state) {
		GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_STATIC).displaystate = state;
		GetGlobalVisibilityOrAdd(OBLMarkerType.SERVER_DYNAMIC).displaystate = state;
		GetGlobalVisibilityOrAdd(OBLMarkerType.GROUP_PING).displaystate = state;
		GetGlobalVisibilityOrAdd(OBLMarkerType.GROUP_MARKER).displaystate = state;
		GetGlobalVisibilityOrAdd(OBLMarkerType.GROUP_PLAYER_MARKER).displaystate = state;
		GetGlobalVisibilityOrAdd(OBLMarkerType.PRIVATE_MARKER).displaystate = state;
	}
	
	string GetCurrentStateName() {
		if (globalVisiblityState == 0) {
			return "усе видно";
		} else if (globalVisiblityState == 1) {
			return "без серверних маркерів";
		} else if (globalVisiblityState == 2) {
			return "лише маркери групи";
		} else if (globalVisiblityState == 3) {
			return "лише маркери гравців";
		} else if (globalVisiblityState == 4) {
			return "лише серверні маркери";
		}
		return "усе приховано";
	}

}
class OBLMarkerVisibilityEntry {

	int uid;
	int displaystate; // 0 3D+2D // 1 2D // 2 None
	
	int GetNextState() {
		displaystate++;
		displaystate = displaystate % 3;
		OBLMarkerVisibilityManager.Get().Save();
		return displaystate;
	}

}
class OBLGlobalVisibilityEntry {

	OBLMarkerType type;
	int displaystate; // 0 3D+2D // 1 2D // 2 None
	
	int GetNextState() {
		displaystate++;
		displaystate = displaystate % 3;
		return displaystate;
	}

}
