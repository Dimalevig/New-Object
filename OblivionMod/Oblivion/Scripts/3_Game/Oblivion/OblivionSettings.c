class OblivionRadiatorRepairSettings
{
	bool  Enabled               = true;
	float DurationSeconds       = 15;    // repair time
	float RepairToHealthPercent = 70;    // radiator health after repair, % of max (70 = Worn)
	bool  AllowRuined           = false; // can a ruined radiator be repaired
	bool  RequireAccessible     = true;  // installed radiator: hood must allow access (same rule as detaching it)
	int   RepairsPerEpoxy       = 2;     // how many repairs one full Epoxy Putty gives

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(Enabled);
		ctx.Write(DurationSeconds);
		ctx.Write(RepairToHealthPercent);
		ctx.Write(AllowRuined);
		ctx.Write(RequireAccessible);
		ctx.Write(RepairsPerEpoxy);
	}

	bool Read(ParamsReadContext ctx)
	{
		return ctx.Read(Enabled)
			&& ctx.Read(DurationSeconds)
			&& ctx.Read(RepairToHealthPercent)
			&& ctx.Read(AllowRuined)
			&& ctx.Read(RequireAccessible)
			&& ctx.Read(RepairsPerEpoxy);
	}
}

// Server-side settings, stored in <server profile>/Oblivion/settings.json.
// Clients receive a copy on connect (OBLIVION_RPC_SETTINGS) so action conditions match the server.
// Each new mechanic adds its own block here.
class OblivionSettings
{
	int ConfigVersion = 1;
	ref OblivionRadiatorRepairSettings RadiatorRepair = new OblivionRadiatorRepairSettings();

	private static ref OblivionSettings s_Instance;

	static OblivionSettings Get()
	{
		if (!s_Instance)
		{
			if (GetGame().IsServer())
				s_Instance = Load();
			else
				s_Instance = new OblivionSettings(); // defaults until the server sends its copy
		}
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

		if (!settings.RadiatorRepair)
			settings.RadiatorRepair = new OblivionRadiatorRepairSettings();

		// Re-save so newly added fields appear in the file with default values.
		JsonFileLoader<OblivionSettings>.JsonSaveFile(OBLIVION_SETTINGS_FILE, settings);
		return settings;
	}

	void WriteSync(ParamsWriteContext ctx)
	{
		RadiatorRepair.Write(ctx);
	}

	static void ReadSync(ParamsReadContext ctx)
	{
		OblivionSettings settings = new OblivionSettings();
		if (!settings.RadiatorRepair.Read(ctx))
		{
			OblivionLog("failed to read settings from server");
			return;
		}
		s_Instance = settings;
		OblivionLog("settings received from server");
	}
}
