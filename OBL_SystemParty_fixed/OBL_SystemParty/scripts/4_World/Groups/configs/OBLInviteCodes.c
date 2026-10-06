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
