// Server-side settings, stored in <server profile>/Oblivion/settings.json.
// Each new mechanic adds its own block of fields here.
class OblivionSettings
{
	int ConfigVersion = 1;

	private static ref OblivionSettings s_Instance;

	static OblivionSettings Get()
	{
		if (!s_Instance)
			s_Instance = Load();
		return s_Instance;
	}

	static OblivionSettings Load()
	{
		OblivionSettings settings = new OblivionSettings();

		if (!FileExist(OBLIVION_PROFILE_DIR))
			MakeDirectory(OBLIVION_PROFILE_DIR);

		if (FileExist(OBLIVION_SETTINGS_FILE))
		{
			JsonFileLoader<OblivionSettings>.JsonLoadFile(OBLIVION_SETTINGS_FILE, settings);
			OblivionLog("settings loaded from " + OBLIVION_SETTINGS_FILE);
		}
		else
		{
			OblivionLog("settings file not found, writing defaults");
		}

		// Re-save so newly added fields appear in the file with default values.
		JsonFileLoader<OblivionSettings>.JsonSaveFile(OBLIVION_SETTINGS_FILE, settings);
		return settings;
	}
}
