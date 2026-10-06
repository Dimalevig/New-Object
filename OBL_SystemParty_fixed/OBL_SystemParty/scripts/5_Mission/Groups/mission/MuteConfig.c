#ifndef OBL_DISABLE_CHAT
class MuteConfig {
	
	const static int VERSION = 1;
	int currentVersion = VERSION;
	
	bool displayGroupTagsInfrontOfName = true;
	bool enableGlobalChatFeature = true;
	bool enableMuteVote = false;
	int muteVoteMinPlayers = 20;
	int muteVoteMuteTimeMins = 10;
	float muteVotePercentile = 0.6;
	ref array<ref ChannelCfg> channels = new array<ref ChannelCfg>();
	ref TStringArray muteAdmins = new TStringArray;
	ref array<ref Param2<string, int>> mutedPlayers = new array<ref Param2<string, int>>();
	ref array<ref PrefixGroup> prefixGroups = new array<ref PrefixGroup>();
	ref TStringArray badWords = new TStringArray();
	bool enabledBadWordsCensor = false;
	bool blockBadWordContainingMessages = false;
	string badWordsBlockedMessage = "Ваше повідомлення містить нецензурні слова!";
	int badWordsMuteTime = 0;
	[NonSerialized()]
	ref TStringArray mutedPlayersSteamids = new TStringArray();
	
	void SendMuteList() {
		UpdateList();
		array<PlayerIdentity> identities = new array<PlayerIdentity>();
		GetGame().GetPlayerIndentities(identities);
		GetGame().RPCSingleParam(null, OBLPartyRPCs.OBL_GLOBAL_MUTELIST, new Param1<bool>(false), true); // Unmute All
		foreach (PlayerIdentity ident : identities) {
			if (!ident)
				continue;
			string steamid = ident.GetPlainId();
			if (mutedPlayersSteamids.Find(steamid) != -1) {
				SendMute(ident); 
			}
		}
	}
	bool IsAdmin(string steamid) {
		return muteAdmins.Find(steamid) != -1;
	}
	
	void MutePlayer(string steamid, int minutes) {
		int now = JMDate.Now(true).GetTimestamp() + minutes * 60;
		UnMutePlayer(steamid);
		mutedPlayers.Insert(new Param2<string, int>(steamid, now));
		SendMuteList();
	}
	
	bool IsMuted(string steamid) {
		for (int i = 0; i < mutedPlayers.Count(); i++) {
			Param2<string, int> muteParams = mutedPlayers.Get(i);
			if (!muteParams) {
				mutedPlayers.Remove(i);
				i--;
				continue;
			} else if (muteParams.param1 == steamid) {
				return true;
			}
		}
		return false;
	}
	
	void UnMutePlayer(string steamid) {
		for (int i = 0; i < mutedPlayers.Count(); i++) {
			Param2<string, int> muteParams = mutedPlayers.Get(i);
			if (!muteParams) {
				mutedPlayers.Remove(i);
				i--;
				continue;
			} else if (muteParams.param1 == steamid) {
				mutedPlayers.Remove(i);
				SendMuteList();
				return;
			}
		}
		SendMuteList();
	}
	
	void SendMute(PlayerIdentity ident) {
		GetGame().RPCSingleParam(null, OBLPartyRPCs.OBL_GLOBAL_MUTELIST, new Param1<bool>(true), true, ident);
	}
	
	void SendChatList(PlayerIdentity ident = null) {
		ScriptRPC rpc = new ScriptRPC();
		if (!enableGlobalChatFeature) {
			rpc.Write(-1);
		} else {
			rpc.Write(channels.Count());
			foreach (ChannelCfg cfg : channels) {
				cfg.WriteToCtx(rpc);
			}
		}
		rpc.Send(null, OBLPartyRPCs.OBL_GLOBAL_CHANNELS, true, ident);
	}
	
	void UpdateList() {
		int timeNow = JMDate.Now(true).GetTimestamp();
		mutedPlayersSteamids.Clear();
		for (int i = 0; i < mutedPlayers.Count(); i++) {
			Param2<string, int> muteParams = mutedPlayers.Get(i);
			if (!muteParams) {
				mutedPlayers.Remove(i);
				i--;
				continue;
			}
			if (timeNow > muteParams.param2) {
				mutedPlayers.Remove(i);
				i--;
				continue;
			}
			mutedPlayersSteamids.Insert(muteParams.param1);
		}
	}
	
	void SaveConfig() {
		OBLFile<MuteConfig>.SaveToJson("ChatConfig.json", this);
	}
	
	PrefixGroup GetPrefixForSteamid(string steamid) {
		foreach (PrefixGroup grp : prefixGroups) {
			if (grp.IsMember(steamid))
				return grp;
		}
		return null;
	}
	
	ChannelCfg FindChannelByName(string name) {
		string lowerName = "" + name;
		lowerName.ToLower();
		foreach (ChannelCfg cfg : channels) {
			string lowerChannelName = "" + cfg.channelName;
			lowerChannelName.ToLower();
			if (lowerChannelName == lowerName)
				return cfg;
		}
		return null;
	}
}
static ref MuteConfig g_MuteConfig;
static MuteConfig GetMuteConfig() {
	if (!g_MuteConfig) {
		g_MuteConfig = LoadMuteConfig();
	}
	return g_MuteConfig;
}
static MuteConfig LoadMuteConfig() {
	OBLLogger.Debug("LoadMuteConfig()");
	MuteConfig cfg = new MuteConfig();
	if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
		MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
	if (!FileExist(OBLPartyConstants.SAVE_PREFIX + "ChatConfig.json")) {
		PrefixGroup adminPrefix = new PrefixGroup();
		adminPrefix.prefix = "[Admin] ";
		adminPrefix.colorR = 255;
		adminPrefix.colorG = 0;
		adminPrefix.colorB = 0;
		adminPrefix.members.Insert("00000000000000000");
		adminPrefix.members.Insert("інші SteamID");
		cfg.muteAdmins.Insert("00000000000000000");
		cfg.muteAdmins.Insert("інші SteamID");
		cfg.prefixGroups.Insert(adminPrefix);
		cfg.badWords = new TStringArray();
		cfg.badWords.Insert("macaco");
		cfg.badWords.Insert("preto");
		cfg.enableMuteVote = false;
		cfg.muteVoteMinPlayers = 20;
		cfg.muteVoteMuteTimeMins = 10;
		cfg.muteVotePercentile = 0.6;
		cfg.badWordsMuteTime = 10;
		cfg.blockBadWordContainingMessages = false;
		cfg.enabledBadWordsCensor = false;
		cfg.badWordsBlockedMessage = "Ваше повідомлення містить заборонені слова!";
		cfg.channels.Insert(ChannelCfg.Init("Прямий", 255, 255, 255, false, false, true, false));
		cfg.channels.Insert(ChannelCfg.Init("Глобальний", 3, 180, 252, true, false, false, true));
		cfg.channels.Insert(ChannelCfg.Init("Група", 3, 252, 15, false, true, false, false));
		JsonFileLoader<MuteConfig>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + "ChatConfig.json", cfg);
	} else {
		OBLLogger.Debug("JsonLoadFile ChatConfig.json");
		JsonFileLoader<MuteConfig>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + "ChatConfig.json", cfg);
	}
	if (cfg.currentVersion != MuteConfig.VERSION) {
		if (cfg.currentVersion <= 0) {
			cfg.badWords.Insert("macaco");
			cfg.badWords.Insert("preto");
		}
		cfg.currentVersion = MuteConfig.VERSION;
		JsonFileLoader<MuteConfig>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + "ChatConfig.json", cfg);
	}
	return cfg;
}
static void ReloadMuteConfig() {
	g_MuteConfig = LoadMuteConfig();
	g_MuteConfig.SendMuteList();
	g_MuteConfig.SendChatList();
}
class MuteAdmin {
	string steam64id;
	string expiryDate;
	
}
class PrefixGroup {

	string prefix;
	int colorR, colorG, colorB;
	ref TStringArray members = new TStringArray();
	
	int GetColor() {
		return ARGB(255, colorR, colorG, colorB);
	}
	
	bool IsMember(string steamid) {
		return members.Find(steamid) != -1;
	}
}
class ChannelCfg {
	string channelName;
	ref OBLColorConfig channelColor;
	bool globalChannel = true;
	bool groupChannel = true;
	bool directChannel = true;
	bool defaultChannel = false;
	[NonSerialized()]
	bool muted = false;
	
	static ChannelCfg Init(string name, int r, int g, int b, bool globalC, bool groupC, bool directC, bool def) {
		ChannelCfg cfg = new ChannelCfg();
		cfg.channelName = name;
		cfg.channelColor = OBLColorConfig.Init(255, r,g,b);
		cfg.globalChannel = globalC;
		cfg.groupChannel = groupC;
		cfg.directChannel = directC;
		cfg.defaultChannel = def;
		return cfg;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(channelName);
		channelColor.WriteToCtx(ctx);
		ctx.Write(globalChannel);
		ctx.Write(groupChannel);
		ctx.Write(directChannel);
		ctx.Write(defaultChannel);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(channelName))
			return false;
		channelColor = new OBLColorConfig();
		if (!channelColor.ReadFromCtx(ctx))
			return false;
		if (!ctx.Read(globalChannel))
			return false;
		if (!ctx.Read(groupChannel))
			return false;
		if (!ctx.Read(directChannel))
			return false;
		if (!ctx.Read(defaultChannel))
			return false;
		return true;
	}
}
#endif
