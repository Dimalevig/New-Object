class OBLPartyLevel {

	int level;
	int maxPlayerCount;
	int subgroupCount;
	int subgroupSize;
	int groupMarkerLimitAdded;
	int groupPlotpolesLimitAdded = 0;
	
	static OBLPartyLevel InitLevel(int lvl, int maxPlayers, int subCount, int subSize, int markerAdded, int plotpoleAdded) {
		OBLPartyLevel lev = new OBLPartyLevel();
		lev.level = lvl;
		lev.maxPlayerCount = maxPlayers;
		lev.subgroupCount = subCount;
		lev.subgroupSize = subSize;
		lev.groupMarkerLimitAdded = markerAdded;
		lev.groupPlotpolesLimitAdded = plotpoleAdded;
		return lev;
	}
	
	bool HasNextLevel() {
		return GetNextLevel() != null;
	}
	
	OBLPartyLevel GetNextLevel() {
		return OBLPartyLevels.Get().GetNextLevel(this);
	}
	
}
class OBLPartyLevels {

	ref array<ref OBLPartyLevel> allLevels = new array<ref OBLPartyLevel>();
	
	static ref OBLPartyLevels g_OBLPartyGroups;
	
	static OBLPartyLevels Get() {
		if (!g_OBLPartyGroups) {
			g_OBLPartyGroups = Load();
		}
		return g_OBLPartyGroups;
	}

	static void Delete() {
		if (g_OBLPartyGroups)
			delete g_OBLPartyGroups;
	}
	
	static OBLPartyLevels Load() {
		OBLPartyLevels levels;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_LEVELS)) {
			levels = LoadDefault();
			JsonFileLoader<array<ref OBLPartyLevel>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_LEVELS, levels.allLevels);
			return levels;
		}
		levels = new OBLPartyLevels();
		JsonFileLoader<array<ref OBLPartyLevel>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_LEVELS, levels.allLevels);
		return levels;
	}
	
	static OBLPartyLevels LoadDefault() {
		OBLPartyLevels def = new OBLPartyLevels;
		def.allLevels.Insert(OBLPartyLevel.InitLevel(0, 20, 6, 5, 30, 2)); // +4
		return def;
	}
	
	OBLPartyLevel FindLevelByUID(int level) {
		if (level == -1)
			return null;
		foreach (OBLPartyLevel level2 : allLevels) {
			if (level2.level == level)
				return level2;
		}
		return null;
	}

	OBLPartyLevel GetHighestLevel() {
		OBLPartyLevel highest = null;
		foreach (OBLPartyLevel level2 : allLevels) {
			if (!highest || level2.level > highest.level)
				highest = level2;
		}
		return highest;
	}
	
	OBLPartyLevel GetNextLevel(OBLPartyLevel level) {
		if (!level)
			return null;
		return FindLevelByUID(level.level + 1);
	}
		
	OBLPartyLevel GetNextLevel(int level) {
		return GetNextLevel(FindLevelByUID(level));
	}
	
}