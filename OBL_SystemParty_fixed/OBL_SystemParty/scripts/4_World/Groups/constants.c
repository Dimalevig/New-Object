class OBLPartyConstants {
	const string SAVE_PREFIX = "$profile:OBLParty/";
	const string SAVE_SUFFIX_GROUP_PERMISSIONS = "Permissions.json";
	const string SAVE_SUFFIX_MAIN_CONFIG = "MainConfig.json";
	const string SAVE_SUFFIX_GROUP_LEVELS = "Levels.json";
	const string SAVE_SUFFIX_STATIC_MARKER = "StaticMarkers.json";
	const string SAVE_SUFFIX_PRIVATE_MARKER = "PrivateMarkers.json";
	const string SAVE_SUFFIX_PRIVATE_MARKER_STATES = "PrivateMarkerDisplaystates.json";
	const string SAVE_SUFFIX_LAYOUT_MANAGER = "LayoutManager.json";
	const string SAVE_SUFFIX_LAYOUT_MANAGER_TEMP = "LayoutManager_TEMP.json";
	const string SAVE_SUFFIX_COLOR_MANAGER = "ColorManager.json";
	const string SAVE_SUFFIX_POSITION_MANAGER = "PositionManager.json";
	const string SAVE_SUFFIX_PLOTPOLE_CONFIG = "PlotpoleConfig.json";
	const string SAVE_SUFFIX_CLANCLOTHING_CONFIG = "ClanClothing.json";
	const string SAVE_SUFFIX_GROUPS_FOLDER = "Partys/";
	const string SAVE_SUFFIX_GROUPSDELETED_FOLDER = "Partys_Deleted/";
	// підгрупи: лише «Онлайн» та «Офлайн», гравець потрапляє туди автоматично
	const int SUBGROUP_ONLINE = 0;
	const int SUBGROUP_OFFLINE = 1;
	const int SUBGROUP_COUNT = 2;
	// розмір групи: 6 за замовчуванням, адмін може задати до 15
	const int DEFAULT_MAX_PLAYERS = 6;
	const int MAX_PLAYERS_LIMIT = 15;
}