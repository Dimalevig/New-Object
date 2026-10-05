// Custom Palisade - global constants (3_Game).
// Only plain constants live here; gameplay classes live in 4_World.

const string CP_MOD_NAME    = "CustomPalisade";
const string CP_MOD_VERSION = "0.1.1";
const string CP_LOG_PREFIX  = "[CustomPalisade] ";

// Class names
const string CP_CLASS_PALISADE_WALL = "CP_PalisadeWall";

// TEST BUILD ONLY: spawn one finished palisade wall in front of a player
// when they join, if there is no wall nearby yet. Set to false for release.
const bool  CP_DEBUG_SPAWN_TEST_WALL         = true;
const float CP_DEBUG_TEST_WALL_DISTANCE      = 4.0;   // metres in front of the player
const float CP_DEBUG_TEST_WALL_SEARCH_RADIUS = 30.0;  // no new wall if one is this close
const int   CP_DEBUG_TEST_WALL_DELAY_MS      = 5000;  // wait until the player is in the world
