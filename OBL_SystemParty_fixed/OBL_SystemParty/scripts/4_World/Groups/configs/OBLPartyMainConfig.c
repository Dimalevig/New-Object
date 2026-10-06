class OBLPartyMainConfig {
	const static int CURRENT_VERSION = 9;
	int configVersion = CURRENT_VERSION;
	string serverLogoPath = "";
	bool canSeeOwnPlayerOnMap = true;
	bool enableKOTHMarkers = true;
	string ownPlayerIconPath = "";
	string otherPlayerIconsPath = "";
	int inactiveGroupLifetimeDays = 30;
	int tacticalPingLifetimeSeconds = 8;
	int inviteCooldownSeconds = 120;
	float inviteMaxDistance = -1;
	bool inviteActionEnabled = false;
	bool inviteActionShowName = false;
	ref array<ref MarkerConfigEntry> markerConfig = new array<ref MarkerConfigEntry>();
	ref TStringArray availableIcons = new TStringArray();
	string customMapLayoutFolder = "";
	string customPlayerlistLayoutFolder = "";
	string customCompassLayoutFolder = "";
	ref array<ref LayoutStyleEntry> layoutStyles = new array<ref LayoutStyleEntry>();
	bool enableCompassHud = false;
	bool enablePlayerList = true;
	bool enablePlayerListDistance = true;
	bool enableSubGroups = true;
	bool enableShop = true;
	string shopApiUrl = "https://oblivion-store.vn.ua";
	int shopServerId = 1;
	string shopSecretKey = "";
	bool enableSubGroupSharedPlayerMapMarker = false;
	bool enableSubGroupSharedPingMapMarker = false;
	bool enableInfoPanelSurvivorCount = true;
	bool enableInfoPanelCursorCoordinates = true;
	bool enableInfoPanelIngameTime = true;
	bool enableInfoPanelRealTime = true;
	bool disableInfoPanelModCreatorMention = false;
	bool disableLoggerDebug = false;
	int groupMarkerLimit = 20;
	ref TStringArray adminSteamids = new TStringArray();
	float offlinePlayer3dMarkerDistance = 20.0;
	ref TStringArray subGroupNames = new TStringArray();
	ref array<ref OBLButtonConfig> buttonConfig = new array<ref OBLButtonConfig>();
	[NonSerialized()]
	int groupCreationCost = 0;
	
	static ref OBLPartyMainConfig g_OBLPartyMainConfig;
	
	static void Delete() {
		if (g_OBLPartyMainConfig)
			delete g_OBLPartyMainConfig;
	}
	
	static OBLPartyMainConfig Get() {
		if (!g_OBLPartyMainConfig) {
			if (GetGame().IsServer() && !OBLFilePlus.IsClient()) {
				OBLLogger.Debug("OBLPartyMainConfig GetGame().IsServer()");
				g_OBLPartyMainConfig = Load();
			} else {
				g_OBLPartyMainConfig = new OBLPartyMainConfig();
				OBLLogger.Debug("OBLPartyMainConfig OBLPartyMainConfig()");
				GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_MAIN, new Param1<bool>(true), true);
			}
		}
		return g_OBLPartyMainConfig;
	}
	
	static OBLPartyMainConfig Load() {
		OBLPartyMainConfig cfg;
		if (!OBLFilePlus.JsonExist(OBLPartyConstants.SAVE_SUFFIX_MAIN_CONFIG)) {
			cfg = LoadDefault();			
			OBLFile<OBLPartyMainConfig>.SaveToJson(OBLPartyConstants.SAVE_SUFFIX_MAIN_CONFIG, cfg);
			return cfg;
		}
		cfg = new OBLPartyMainConfig();
		OBLFile<OBLPartyMainConfig>.LoadFromJson(OBLPartyConstants.SAVE_SUFFIX_MAIN_CONFIG, cfg);
		if (cfg.configVersion != CURRENT_VERSION) {
			UpgradeConfigFile(cfg);
			OBLFile<OBLPartyMainConfig>.SaveToJson(OBLPartyConstants.SAVE_SUFFIX_MAIN_CONFIG, cfg);
		}
		EnsureButtonConfig(cfg);
		// підгрупи «Онлайн»/«Офлайн» — обов'язкова частина системи груп
		cfg.enableSubGroups = true;
		return cfg;
	}
	
	static void UpgradeConfigFile(OBLPartyMainConfig cfg) {
		OBLLogger.Debug("Upgrading MainConfig from Version " + cfg.configVersion + " to Version " + CURRENT_VERSION);
		if (cfg.configVersion < 3) {
			cfg.layoutStyles = new array<ref LayoutStyleEntry>();
			LayoutStyleEntry lEntry = new LayoutStyleEntry();
			lEntry.styles.Insert("default");
			lEntry.entryName = "інтерфейс мапи";
			cfg.layoutStyles.Insert(lEntry);
			
			lEntry = new LayoutStyleEntry();
			lEntry.styles.Insert("default");
			lEntry.entryName = "інтерфейс компаса";
			cfg.layoutStyles.Insert(lEntry);
			
			lEntry = new LayoutStyleEntry();
			lEntry.styles.Insert("default");
			lEntry.styles.Insert("small");
			lEntry.styles.Insert("tiny");
			lEntry.entryName = "Список гравців";
			cfg.layoutStyles.Insert(lEntry);
		}
		if (cfg.configVersion < 4) {
			cfg.enablePlayerListDistance = true;
		}
		if (cfg.configVersion < 5) {
			cfg.inviteMaxDistance = -1;
			cfg.inviteActionEnabled = false;
			cfg.inviteActionShowName = false;
		}
		if (cfg.configVersion < 6) {
			cfg.enableKOTHMarkers = true;
		}
		if (cfg.configVersion < 7) {
			EnsureButtonConfig(cfg);
		}
		if (cfg.configVersion < 9) {
			cfg.enableShop = true;
			cfg.shopApiUrl = "https://oblivion-store.vn.ua";
			cfg.shopServerId = 1;
			cfg.shopSecretKey = "";
		}
		cfg.configVersion = CURRENT_VERSION;
	}

	static void EnsureButtonConfig(OBLPartyMainConfig cfg) {
		if (!cfg.buttonConfig)
			cfg.buttonConfig = new array<ref OBLButtonConfig>();
		while (cfg.buttonConfig.Count() < 2) {
			cfg.buttonConfig.Insert(GetDefaultButtonConfig(cfg.buttonConfig.Count()));
		}
		for (int i = 0; i < cfg.buttonConfig.Count(); i++) {
			if (!cfg.buttonConfig.Get(i))
				cfg.buttonConfig.Set(i, GetDefaultButtonConfig(i));
		}
		OBLButtonConfig discord = cfg.buttonConfig.Get(0);
		if (discord && (discord.link == "" || discord.link == "google.com")) {
			discord.buttonName = "Discord";
			discord.subtext = "Мій Discord";
			discord.link = "https://discord.gg/your-server";
		}
		OBLButtonConfig donate = cfg.buttonConfig.Get(1);
		if (donate && (donate.link == "" || donate.link == "seu.link.aqui")) {
			donate.buttonName = "Пожертви";
			donate.subtext = "Пожертви переказом";
			donate.link = "https://your-donate-link.example";
		}
	}

	static OBLButtonConfig GetDefaultButtonConfig(int index) {
		if (index == 0)
			return OBLButtonConfig.InitButton("Discord", "Мій Discord", "https://discord.gg/your-server");
		if (index == 1)
			return OBLButtonConfig.InitButton("Пожертви", "Пожертви переказом", "https://your-donate-link.example");
		return OBLButtonConfig.InitButton("", "", "");
	}
	
	static OBLPartyMainConfig LoadDefault() {
		OBLLogger.Debug("OBLPartyMainConfig LoadDefault");
		OBLPartyMainConfig def = new OBLPartyMainConfig();
		def.configVersion = CURRENT_VERSION;
		def.serverLogoPath = "";
		def.canSeeOwnPlayerOnMap = true;
		def.inactiveGroupLifetimeDays = 30;
		def.ownPlayerIconPath = "OBL_SystemParty\\gui\\icons\\player.paa";
		def.otherPlayerIconsPath = "OBL_SystemParty\\gui\\icons\\player.paa";
		def.customMapLayoutFolder = "";
		def.customPlayerlistLayoutFolder = "";
		def.customCompassLayoutFolder = "";
		
		def.layoutStyles = new array<ref LayoutStyleEntry>();
		LayoutStyleEntry lEntry = new LayoutStyleEntry();
		lEntry.styles.Insert("default");
		lEntry.entryName = "інтерфейс мапи";
		def.layoutStyles.Insert(lEntry);
		
		lEntry = new LayoutStyleEntry();
		lEntry.styles.Insert("default");
		lEntry.entryName = "інтерфейс компаса";
		def.layoutStyles.Insert(lEntry);
		
		lEntry = new LayoutStyleEntry();
		lEntry.styles.Insert("default");
		lEntry.styles.Insert("small");
		lEntry.styles.Insert("tiny");
		lEntry.entryName = "Список гравців";
		def.layoutStyles.Insert(lEntry);
		
		def.enableKOTHMarkers = true;
		def.enableCompassHud = true;
		def.enablePlayerList = true;
		def.enablePlayerListDistance = true;
		def.enableSubGroups = true;
		def.enableSubGroupSharedPlayerMapMarker = false;
		def.enableSubGroupSharedPingMapMarker = false;
		def.enableInfoPanelSurvivorCount = true;
		def.enableInfoPanelCursorCoordinates = true;
		def.enableInfoPanelIngameTime = true;
		def.enableInfoPanelRealTime = true;
		def.disableInfoPanelModCreatorMention = false;
		def.disableLoggerDebug = false;
		def.enableShop = true;
		def.shopApiUrl = "https://oblivion-store.vn.ua";
		def.shopServerId = 1;
		def.shopSecretKey = "";
		def.offlinePlayer3dMarkerDistance = 20.0;
		def.adminSteamids.Insert("00000000000000000");
		def.subGroupNames.Insert("Онлайн");
		def.subGroupNames.Insert("Офлайн");
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.SERVER_STATIC, -1, true, true, true, false));
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.SERVER_DYNAMIC, -1, true, true, true, false));
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.GROUP_PING, -1, true, false, false, true));
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.GROUP_MARKER, -1, true, true, true, false));
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.PRIVATE_MARKER, -1, true, true, true, false));
		def.markerConfig.Insert(MarkerConfigEntry.Init(OBLMarkerType.GROUP_PLAYER_MARKER, 2000, true, false, true, true));
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\marker.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\marker-stroked.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\cross.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\home.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\camp.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\hospital.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\flag.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\star.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\car.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\parking.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\heli.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\rail.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\ship.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\scooter.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\bank.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\restaurant.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\post.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\castle.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\ranger-station.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\water.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\triangle.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\cow.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\bear.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\car-repair.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\communications.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\roadblock.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\stadium.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\skull.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\rocket.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\bbq.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\ping.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\player.paa");
		def.availableIcons.Insert("OBL_SystemParty\\gui\\icons\\fire.paa");
		EnsureButtonConfig(def);
		return def;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(serverLogoPath);
		ctx.Write(canSeeOwnPlayerOnMap);
		ctx.Write(enableKOTHMarkers);
		ctx.Write(ownPlayerIconPath);
		ctx.Write(otherPlayerIconsPath);
		ctx.Write(customMapLayoutFolder);
		ctx.Write(customPlayerlistLayoutFolder);
		ctx.Write(customCompassLayoutFolder);
		ctx.Write(layoutStyles.Count());
		foreach (LayoutStyleEntry style : layoutStyles) {
			ctx.Write(style.entryName);
			ctx.Write(style.styles.Count());
			foreach (string stl : style.styles)
				ctx.Write(stl);
		}
		ctx.Write(enableCompassHud);
		ctx.Write(enablePlayerList);
		ctx.Write(enablePlayerListDistance);
		ctx.Write(enableSubGroups);
		ctx.Write(enableSubGroupSharedPlayerMapMarker);
		ctx.Write(enableSubGroupSharedPingMapMarker);
		ctx.Write(enableInfoPanelSurvivorCount);
		ctx.Write(enableInfoPanelCursorCoordinates);
		ctx.Write(enableInfoPanelIngameTime);
		ctx.Write(enableInfoPanelRealTime);
		ctx.Write(disableInfoPanelModCreatorMention);
		ctx.Write(disableLoggerDebug);
		ctx.Write(offlinePlayer3dMarkerDistance);
		ctx.Write(subGroupNames.Count());
		foreach (string str : subGroupNames) {
			ctx.Write(str);
		}
		ctx.Write(markerConfig.Count());
		foreach (MarkerConfigEntry entry : markerConfig) {
			entry.WriteToCtx(ctx);
		}
		ctx.Write(availableIcons.Count());
		foreach (string s : availableIcons) {
			ctx.Write(s);
		}
		ctx.Write(buttonConfig.Count());
		foreach (OBLButtonConfig btnCfg : buttonConfig) {
			btnCfg.WriteToCtx(ctx);
		}
		ctx.Write(groupCreationCost);
		ctx.Write(enableShop);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(serverLogoPath))
			return false;
		if (!ctx.Read(canSeeOwnPlayerOnMap))
			return false;
		if (!ctx.Read(enableKOTHMarkers))
			return false;
		if (!ctx.Read(ownPlayerIconPath))
			return false;
		if (!ctx.Read(otherPlayerIconsPath))
			return false;
		if (!ctx.Read(customMapLayoutFolder))
			return false;
		if (!ctx.Read(customPlayerlistLayoutFolder))
			return false;
		if (!ctx.Read(customCompassLayoutFolder))
			return false;
		int count1 = 0;
		if (!ctx.Read(count1))
			return false;
		layoutStyles.Clear();
		for (int i = 0; i < count1; i++) {
			string name;
			if (!ctx.Read(name))
				return false;
			int count2 = 0;
			if (!ctx.Read(count2))
				return false;
			LayoutStyleEntry lEntry = new LayoutStyleEntry();
			lEntry.entryName = name;
			for (int a = 0; a < count2; a++) {
				string stl;
				if (!ctx.Read(stl))
					return false;
				lEntry.styles.Insert(stl);
			}
			layoutStyles.Insert(lEntry);
		}
		if (!ctx.Read(enableCompassHud))
			return false;
		if (!ctx.Read(enablePlayerList))
			return false;
		if (!ctx.Read(enablePlayerListDistance))
			return false;
		if (!ctx.Read(enableSubGroups))
			return false;
		if (!ctx.Read(enableSubGroupSharedPlayerMapMarker))
			return false;
		if (!ctx.Read(enableSubGroupSharedPingMapMarker))
			return false;
		if (!ctx.Read(enableInfoPanelSurvivorCount))
			return false;
		if (!ctx.Read(enableInfoPanelCursorCoordinates))
			return false;
		if (!ctx.Read(enableInfoPanelIngameTime))
			return false;
		if (!ctx.Read(enableInfoPanelRealTime))
			return false;
		if (!ctx.Read(disableInfoPanelModCreatorMention))
			return false;
		if (!ctx.Read(disableLoggerDebug))
			return true;
		if (!ctx.Read(offlinePlayer3dMarkerDistance))
			return false;
		int count = 0;
		if (!ctx.Read(count))
			return false;
		subGroupNames.Clear();
		for (i = 0; i < count; i++) {
			string str;
			if (!ctx.Read(str))
				return false;
			subGroupNames.Insert(str);
		}
		count = 0;
		if (!ctx.Read(count))
			return false;
		markerConfig.Clear();
		for (i = 0; i < count; i++) {
			MarkerConfigEntry entry = new MarkerConfigEntry();
			if (!entry.ReadFromCtx(ctx))
				return false;
			markerConfig.Insert(entry);
		}
		if (!ctx.Read(count))
			return false;
		availableIcons.Clear();
		for (i = 0; i < count; i++) {
			string path;
			if (!ctx.Read(path))
				return false;
			availableIcons.Insert(path);
		}
		if (!ctx.Read(count))
			return false;
		buttonConfig.Clear();
		for (i = 0; i < count; i++) {
			OBLButtonConfig cfg2 = new OBLButtonConfig();
			if (!cfg2.ReadFromCtx(ctx))
				return false;
			buttonConfig.Insert(cfg2);
		}
		EnsureButtonConfig(this);
		if (!ctx.Read(groupCreationCost))
			return false;
		// Append-only fields (v9+): if old server doesn't send them, keep defaults.
		if (!ctx.Read(enableShop))
			return true;
		return true;
	}
	
	void RPC_OBL(PlayerIdentity sender, ParamsReadContext ctx) {
		if (GetGame().IsServer()) {
			ScriptRPC rpc = new ScriptRPC();
			WriteToCtx(rpc);
			rpc.Send(null, OBLPartyRPCs.CONFIG_SYNC_MAIN, true, sender);
		} else {
			if (!ReadFromCtx(ctx)) {
				OBLLogger.Debug("Unable to read Main Config from Server !");
				return;
			}
			OBLLogger.Debug("Successfully Received Main Config from Server");
		}
	}
	
	MarkerConfigEntry GetMarkerConfigEntry(OBLMarkerType type) {
		foreach (MarkerConfigEntry entry : markerConfig) {
			if (entry.type == type)
				return entry;
		}
		return null;
	}
	
	// підгруп лише дві, назви фіксовані
	string GetSubGroupName(int index) {
		if (index == OBLPartyConstants.SUBGROUP_OFFLINE)
			return "Офлайн";
		return "Онлайн";
	}
	
	bool IsAdmin(string steamid) {
		return adminSteamids.Find(steamid) != -1;
	}
	
	bool IsAdmin(PlayerIdentity ident) {
		if (!ident)
			return false;
		return IsAdmin(ident.GetPlainId());
	}
	
	bool IsAdmin(PlayerBase player) {
		if (!player)
			return false;
		return IsAdmin(player.GetIdentity());
	}
	
	bool IsCompassVisible(OBLMarkerType type) {
		MarkerConfigEntry cfgEntry = GetMarkerConfigEntry(type);
		if (!cfgEntry)
			return false;
		return cfgEntry.displayCompass;
	}
	
	void PrintMarkerConfigEntries() {
		OBLLogger.Debug("Marker config Entires: " + markerConfig.Count());
		foreach (MarkerConfigEntry entry : markerConfig) {
			OBLLogger.Debug("" + entry.type + " " + entry.maxDistance + " " + entry.display3d + " "  + entry.displayDistance + " "  + entry.displayMap);
		}
	}
}
class MarkerConfigEntry {
	
	OBLMarkerType type;
	string typeString;
	int maxDistance;
	bool display3d;
	bool displayDistance;
	bool displayMap;
	bool displayCompass;
	
	static MarkerConfigEntry Init(OBLMarkerType type2, int maxDist, bool disp3d, bool dispDist, bool dispMap, bool dispComp) {
		MarkerConfigEntry ent = new MarkerConfigEntry;
		ent.type = type2;
		ent.typeString = typename.EnumToString(OBLMarkerType, type2);
		ent.maxDistance = maxDist;
		ent.display3d = disp3d;
		ent.displayDistance = dispDist;
		ent.displayMap = dispMap;
		ent.displayCompass = dispComp;
		return ent;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(type);
		ctx.Write(maxDistance);
		ctx.Write(display3d);
		ctx.Write(displayDistance);
		ctx.Write(displayMap);
		ctx.Write(displayCompass);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(type))
			return false;
		if (!ctx.Read(maxDistance))
			return false;
		if (!ctx.Read(display3d))
			return false;
		if (!ctx.Read(displayDistance))
			return false;
		if (!ctx.Read(displayMap))
			return false;
		if (!ctx.Read(displayCompass))
			return false;
		return true;
	}
	
}
class LayoutStyleEntry {

	string entryName;
	ref TStringArray styles = new TStringArray();

}
