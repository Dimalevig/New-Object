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
