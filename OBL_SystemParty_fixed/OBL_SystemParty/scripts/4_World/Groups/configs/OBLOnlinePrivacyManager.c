class OBLOnlinePrivacyManager {
	static ref TStringArray hiddenSteamids = new TStringArray();
	
	static void ReadHiddenSteamids(ParamsReadContext ctx) {
		Ensure();
		hiddenSteamids.Clear();
		int count = 0;
		if (!ctx.Read(count))
			return;
		for (int i = 0; i < count; i++) {
			string steamid;
			if (!ctx.Read(steamid))
				return;
			if (steamid != "" && hiddenSteamids.Find(steamid) == -1)
				hiddenSteamids.Insert(steamid);
		}
	}
	
	static bool IsHidden(string steamid) {
		Ensure();
		if (steamid == "" || steamid == MissionBaseWorld.mySteamid)
			return false;
		return hiddenSteamids.Find(steamid) != -1;
	}
	
	static void Ensure() {
		if (!hiddenSteamids)
			hiddenSteamids = new TStringArray();
	}
}


// коди запрошення онлайн-гравців (steamid → 5-значний код), надсилає сервер
class OBLInviteCodes {
	static ref map<string, string> codes = new map<string, string>();

	static void Read(ParamsReadContext ctx) {
		if (!codes)
			codes = new map<string, string>();
		int count = 0;
		if (!ctx.Read(count))
			return;
		for (int i = 0; i < count; i++) {
			string steamid, code;
			if (!ctx.Read(steamid) || !ctx.Read(code))
				return;
			if (steamid != "")
				codes.Set(steamid, code);
		}
	}

	static string Get(string steamid) {
		if (!codes || !codes.Contains(steamid))
			return "";
		return codes.Get(steamid);
	}
}
