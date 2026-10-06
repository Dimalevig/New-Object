class OblivionRadiatorRepairSettings
{
	bool  Enabled               = true;
	float DurationSeconds       = 15;    // тривалість ремонту, с
	float RepairToHealthPercent = 70;    // стан радіатора після ремонту, % від максимуму (70 = Worn)
	bool  AllowRuined           = false; // чи можна ремонтувати зіпсований (ruined) радіатор
	bool  RequireAccessible     = true;  // радіатор у машині: потрібен доступ (те саме правило, що й для зняття)
	int   RepairsPerEpoxy       = 2;     // скільки ремонтів дає одна повна епоксидна смола

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

// Серверні налаштування, файл <профіль сервера>/Oblivion/settings.json.
// Клієнт отримує копію при підключенні (OBLIVION_RPC_SETTINGS), щоб умови дій збігались із сервером.
// Кожна нова механіка додає сюди свій блок.
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
				s_Instance = new OblivionSettings(); // дефолти, поки сервер не надішле свої
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
			OblivionLog("налаштування завантажено з " + OBLIVION_SETTINGS_FILE);
		}
		else
		{
			OblivionLog("файл налаштувань не знайдено, записую дефолтні");
		}

		if (!settings.RadiatorRepair)
			settings.RadiatorRepair = new OblivionRadiatorRepairSettings();

		// Перезаписуємо, щоб нові поля з'явились у файлі з дефолтними значеннями.
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
			OblivionLog("не вдалося прочитати налаштування від сервера");
			return;
		}
		s_Instance = settings;
		OblivionLog("налаштування отримано від сервера");
	}
}
