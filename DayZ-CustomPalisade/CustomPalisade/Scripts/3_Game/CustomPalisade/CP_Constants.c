// Custom Palisade - global constants (3_Game).
// Only plain constants live here; gameplay classes live in 4_World.

const string CP_MOD_NAME    = "CustomPalisade";
const string CP_MOD_VERSION = "0.2.1";
const string CP_LOG_PREFIX  = "[CustomPalisade] ";

// Class names
const string CP_CLASS_PALISADE_KIT  = "CP_PalisadeKit";
const string CP_CLASS_PALISADE_WALL = "CP_PalisadeWall";
const string CP_CLASS_PALISADE_WALL_MODEL = "CP_PalisadeWallModel";

// TEST BUILD ONLY: spawn one finished palisade wall in front of a player
// when they join, if there is no wall nearby yet. Set to false for release.
const bool  CP_DEBUG_SPAWN_TEST_WALL         = true;
const float CP_DEBUG_TEST_WALL_DISTANCE      = 4.0;   // metres in front of the player
const float CP_DEBUG_TEST_WALL_SEARCH_RADIUS = 30.0;  // no new wall if one is this close
const int   CP_DEBUG_TEST_WALL_DELAY_MS      = 5000;  // wait until the player is in the world

// TEST BUILD ONLY: place the new log palisade model next to the test wall
// (static object, re-created after a restart). Set to false for release.
const bool  CP_DEBUG_SPAWN_TEST_MODEL_WALL   = true;
const float CP_DEBUG_TEST_MODEL_SIDE_OFFSET  = 6.0;   // metres to the right of the test wall

// TEST BUILD ONLY: drop one palisade kit at the player's feet when they join,
// if there is no kit nearby yet. Set to false for release.
const bool  CP_DEBUG_SPAWN_TEST_KIT          = true;
const float CP_DEBUG_TEST_KIT_SEARCH_RADIUS  = 5.0;   // no new kit if one is this close
