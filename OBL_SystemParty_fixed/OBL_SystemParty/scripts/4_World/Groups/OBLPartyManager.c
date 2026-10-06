class OBLPartyManager {
	
	static ref TStringArray illegalFilenames = new TStringArray();
	static ref TStringArray illegalFilenamesNum = new TStringArray();

	static ref OBLPartyManager g_OBLPartyManager;
	
	static OBLPartyManager Get() {
		if (!g_OBLPartyManager) {
			g_OBLPartyManager = Load();
			
		}
		return g_OBLPartyManager;
	}
	
	static void Delete() {
		if (g_OBLPartyManager)
			delete g_OBLPartyManager;
	}
	
	static OBLPartyManager Load() {
		OBLPartyManager groupMgr = new OBLPartyManager;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER)) {
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER);
		}
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPSDELETED_FOLDER)) {
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPSDELETED_FOLDER);
		}
		groupMgr.LoadAllGroups();
		OBLLogger.Debug("Loaded OBLPartyManager");
		illegalFilenames.Clear();
		illegalFilenames.Insert("CON");
		illegalFilenames.Insert("PRN");
		illegalFilenames.Insert("AUX");
		illegalFilenames.Insert("NUL");
		illegalFilenamesNum.Clear();
		illegalFilenamesNum.Insert("COM");
		illegalFilenamesNum.Insert("LPT");
		illegalFilenames.Insert("LST");
		return groupMgr;
	}
	
	OBLParty GetGroupByHash(int hash) {
		return null;
	}
	
	void LoadAllGroups() {}
	
	bool GroupTagTaken(string tag, OBLParty exception = null) { return false; };
	
	void SaveGroup(OBLParty grp) {}
	void DeleteGroup(OBLParty group) {}
}