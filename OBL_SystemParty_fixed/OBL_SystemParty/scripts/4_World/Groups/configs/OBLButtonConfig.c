class OBLButtonConfig {

	string buttonName, subtext, link;
	
	static OBLButtonConfig InitButton(string name, string sub, string lnk) {
		OBLButtonConfig cfg = new OBLButtonConfig();
		cfg.buttonName = name;
		cfg.subtext = sub;
		cfg.link = lnk;
		return cfg;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(buttonName);
		ctx.Write(subtext);
		ctx.Write(link);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(buttonName))
			return false;
		if (!ctx.Read(subtext))
			return false;
		if (!ctx.Read(link))
			return false;
		return true;
	}
	
}