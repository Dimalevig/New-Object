class OBLLayoutConfig {
	
	int playerlistLayoutIndex = 0;
	bool streamerModeEnabled = false;
	bool hideOnlineStatus = false;
	
	static ref OBLLayoutConfig g_OBLLayoutConfig;
	static ref ScriptInvoker Event_OnLayoutChanged = new ScriptInvoker();
	static ref ScriptInvoker Event_StreamerModeChanged = new ScriptInvoker();
	
	static OBLLayoutConfig Get() {
		if (!g_OBLLayoutConfig) {
			g_OBLLayoutConfig = Load();
		}
		return g_OBLLayoutConfig;
	}
	
	static OBLLayoutConfig Load() {
		OBLLayoutConfig mgr = new OBLLayoutConfig();
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_LAYOUT_MANAGER_TEMP)) {
			mgr.Save();
			return mgr;
		}
		OBLLogger.Debug("JsonLoadFile OBLLayoutConfig.json");
		JsonFileLoader<OBLLayoutConfig>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_LAYOUT_MANAGER_TEMP, mgr);
		// OBL FIX: the "Приховати мій онлайн" checkbox was removed. Force any
		// previously saved value off, otherwise a player who had it enabled
		// would stay invisible forever with no UI left to switch it back.
		mgr.hideOnlineStatus = false;
		return mgr;
	}
	
	void Save() {
		JsonFileLoader<OBLLayoutConfig>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_LAYOUT_MANAGER_TEMP, this);
	}
	
	static void ResetAll() {
		OBLLayoutConfig cfg = OBLLayoutConfig.Get();
		cfg.playerlistLayoutIndex = 0;
		cfg.streamerModeEnabled = false;
		cfg.hideOnlineStatus = false;
		InvokeOnLayoutChanged();
	}
	
	static void Reload() {
		g_OBLLayoutConfig = Load();
		InvokeOnLayoutChanged();
	}
	
	static void InvokeOnLayoutChanged() {
		Event_OnLayoutChanged.Invoke();
	}
	
	void SetStreamerMode(bool enabled) {
		this.streamerModeEnabled = enabled;
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Streamer Mode changed to: " + enabled);
		Event_StreamerModeChanged.Invoke(enabled);
	}
	
	void SetPlayerlistLayout(int index) {
		playerlistLayoutIndex = index;
		InvokeOnLayoutChanged();
	}
	
	string GetCurrentPageLayout(int pageId, int subId = 0) {
		return GetCurrentLayout("Map Page " + pageId + " " + subId);
	}
	
	string GetCurrentLayout(string name) {
		switch (name) {
			case "Compass Marker":
				return "OBL_SystemParty/gui/layouts/compass/compassMarker_default.layout";
			case "Player List":
				if (playerlistLayoutIndex == 0) {
					return "OBL_SystemParty/gui/layouts/playerlist/playerlistentry_default.layout";
				} else if (playerlistLayoutIndex == 1) {
					return "OBL_SystemParty/gui/layouts/playerlist/playerlistentry_small.layout";
				} else if (playerlistLayoutIndex == 2) {
					return "OBL_SystemParty/gui/layouts/playerlist/playerlistentry_tiny.layout";
				}
				return "OBL_SystemParty/gui/layouts/playerlist/playerlistentry_small.layout";
			case "Map Marker Add Popup":
				return "OBL_SystemParty/gui/layouts/mapmenu/markerpopup_default.layout";
			case "Map Top Button":
				return "OBL_SystemParty/gui/layouts/mapmenu/topButton_default.layout";
			case "Map":
				return "OBL_SystemParty/gui/layouts/mapmenu/mapmenu_default.layout";
			case "Map Marker List Entry":
				return "OBL_SystemParty/gui/layouts/mapmenu/markerlist/markerlistentry_default.layout";
			case "Compass":
				return "OBL_SystemParty/gui/layouts/compass/compass_default.layout";
			case "Map Page 0 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_0_0_default.layout";
			case "Map Page 1 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_1_0_default.layout";
			case "Map Page 1 1":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_1_1_default.layout";
			case "Map Page 2 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_2_0_default.layout";
			case "Map Page 3 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_3_0_default.layout";
			case "Map Page 5 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_5_0_default.layout";
			case "Map Page 6 0":
				return "OBL_SystemParty/gui/layouts/mapmenu/pages/page_6_0_default.layout";
		}
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Current Layout of " + name + " not found !");
		return "";
	}
	
}
