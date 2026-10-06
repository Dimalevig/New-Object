const string OBLIVION_MOD_VERSION   = "0.2.0";
const string OBLIVION_PROFILE_DIR   = "$profile:Oblivion/";
const string OBLIVION_SETTINGS_FILE = "$profile:Oblivion/settings.json";

// Server -> client: settings sync on connect.
const int OBLIVION_RPC_SETTINGS = 0x0B1100;

static void OblivionLog(string msg)
{
	Print("[Oblivion] " + msg);
}
