class OBLColorManager {

	static ref OBLColorManager g_OBLColorManager;
	
	ref array<ref Param2<string, int>> colorss = new array<ref Param2<string, int>>();
	string changedColor = "";
	
	static ref ScriptInvoker Event_OnColorChange = new ScriptInvoker();
	// службовий запис у ColorManager.json (не колір, у налаштуваннях не показується)
	static const string PALETTE_KEY = "__oblivion_palette";
	
	static OBLColorManager Get() {
		if (!g_OBLColorManager) {
			g_OBLColorManager = Load();
		}
		return g_OBLColorManager;
	}
	
	static OBLColorManager Load() {
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
		OBLColorManager mgr = new OBLColorManager();
		if (FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_COLOR_MANAGER)) {
			OBLLogger.Debug("JsonLoadFile COLOR_MANAGER.json");
			JsonFileLoader<array<ref Param2<string, int>>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_COLOR_MANAGER, mgr.colorss);
		} 
		// нова палітра Oblivion: один раз замінюємо старі збережені кольори
		if (mgr.GetColor(PALETTE_KEY, 0) != OBLTheme.PALETTE_VERSION) {
			mgr.colorss.Clear();
			mgr.SetColor(PALETTE_KEY, OBLTheme.PALETTE_VERSION);
		}
		mgr.SetDefaultColors();
		mgr.ReplaceColors(mgr.colorss);
		mgr.Save();
	
		return mgr;
	}
	
	static void Reload() {
		g_OBLColorManager = Load();
		InvokeOnChanged();
	}
	
	void Save() {
		ref array<ref Param2<string, int>> colorssave = new array<ref Param2<string, int>>();
		if (!colorss) {
			if (FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_COLOR_MANAGER)) {
				JsonFileLoader<array<ref Param2<string, int>>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_COLOR_MANAGER, colorss);
			}
		}
		
		// Add all Current Colors to the Array
		foreach (Param2<string, int> color : colorss) {
			colorssave.Insert(color);
		}
		// Go through all Colors that were saved in the config to not delete entries from other servers.
		foreach (Param2<string, int> oldColor : colorss) {
			bool found = false;
			// Check if the Color is already present and ignore them
			foreach (Param2<string, int> setColor : colorssave) {
				if (oldColor.param1 == setColor.param1) {
					found = true;
					break;
				}
			}
			// if color was not found, add it to the list
			if (!found)
				colorssave.Insert(oldColor);
		}
		JsonFileLoader<array<ref Param2<string, int>>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_COLOR_MANAGER, colorssave);
	}
	
	private void ReplaceColors(array<ref Param2<string, int>> colors_) {
		foreach (Param2<string, int> color : colors_) {
			SetColor(color.param1, color.param2, false);
		}
	}
	
	void ResetAll() {
		colorss.Clear();
		SetColor(PALETTE_KEY, OBLTheme.PALETTE_VERSION);
		SetDefaultColors();
		InvokeOnChanged();
	}
	
	static void InvokeOnChanged() {
		Event_OnColorChange.Invoke();
	}
	
	int GetColor(string colorStr, int defaultColor = -1) {
		if (changedColor == colorStr) {
			SetColor(colorStr, defaultColor);
			changedColor = "";
		}
		foreach (Param2<string, int> color : colorss) {
			if (color && color.param1 == colorStr)
				return color.param2;
		}
		colorss.Insert(new Param2<string, int>(colorStr, defaultColor));
		return defaultColor;
	}
	
	void SetColor(string colorStr, int colorARGB, bool add = true) {
		foreach (Param2<string, int> color : colorss) {
			if (color && color.param1 == colorStr) {
				color.param2 = colorARGB;
				return;
			}
		}
		if (add)
			colorss.Insert(new Param2<string, int>(colorStr, colorARGB));
	}
	
	void SetDefaultColors() {
		// палітра Oblivion
		GetColor("Player 3D Marker", OBLTheme.AccentLight());
		GetColor("Own Player Map Marker", OBLTheme.Glow());
		GetColor("Player Online", OBLTheme.Accent());
		GetColor("Player Offline", ARGB(150, 142, 136, 172));
		GetColor("Ping 3D Marker", OBLTheme.Glow());
		GetColor("Compass", OBLTheme.Text());
		GetColor("Compass Line", OBLTheme.Accent());
		GetColor("Playerlist entry full health", OBLTheme.AccentLight());
		GetColor("Playerlist entry zero health", OBLTheme.Danger());
		GetColor("Playerlist entry border", ARGB(200, 123, 92, 255));
	}
	
	void ResetColorToDefault(string colorStr) {
		changedColor = colorStr;
		SetDefaultColors();
	}
	
	TStringArray GetColorStrings() {
		ref TStringArray arr = new TStringArray();
		foreach (Param2<string, int> color : colorss) {
			if (color && color.param1 != PALETTE_KEY)
				arr.Insert(color.param1);
		}
		return arr;
	}

}