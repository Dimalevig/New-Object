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

class OblivionMetalPlatesSettings
{
	bool Enabled         = true;
	int  PlatesFromDoor  = 1;  // пластин з дверей
	int  PlatesFromTrunk = 2;  // пластин з кришки багажника
	int  PlatesFromHood  = 2;  // пластин з капота
	int  HacksawDamage   = 20; // скільки здоров'я знімає з пилки за один розпил

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(Enabled);
		ctx.Write(PlatesFromDoor);
		ctx.Write(PlatesFromTrunk);
		ctx.Write(PlatesFromHood);
		ctx.Write(HacksawDamage);
	}

	bool Read(ParamsReadContext ctx)
	{
		return ctx.Read(Enabled)
			&& ctx.Read(PlatesFromDoor)
			&& ctx.Read(PlatesFromTrunk)
			&& ctx.Read(PlatesFromHood)
			&& ctx.Read(HacksawDamage);
	}
}

class OblivionVehicleActionsSettings
{
	bool Enabled = true;
	bool CargoFromInside = true; // інвентар машини зсередини, у русі й при закритому багажнику

	// Дії, дозволені в машині. Назва класу дії — підходять і всі її нащадки
	// (ActionConsume = вся їжа й пиття).
	ref array<string> AllowedActions = {
		"ActionConsume",              // їсти, пити
		"ActionConsumeSingle",        // таблетки, вітаміни
		"ActionBandageSelf",          // бинтуватися
		"ActionSplintSelf",           // шина
		"ActionInjectSelf",           // уколи собі (адреналін, морфін)
		"ActionDisinfectSelf",        // дезінфекція
		"ActionMeasureTemperatureSelf", // градусник
		"ActionLoadMagazine",         // заряджати магазин
		"ActionLoadMagazineQuick",
		"ActionEmptyMagazine",        // розряджати магазин
		"ActionToggleNVG"             // ПНВ
	};

	void Write(ParamsWriteContext ctx)
	{
		ctx.Write(Enabled);
		ctx.Write(CargoFromInside);
		ctx.Write(AllowedActions);
	}

	bool Read(ParamsReadContext ctx)
	{
		return ctx.Read(Enabled)
			&& ctx.Read(CargoFromInside)
			&& ctx.Read(AllowedActions);
	}
}

// Тільки серверні, клієнту не передаються.
class OblivionBountySettings
{
	bool  Enabled           = true;
	int   KillsForBounty    = 5;     // вбивств поспіль без смерті, щоб отримати баунті
	bool  AnnounceEveryKill = true;  // після баунті — оголошувати кожне нове вбивство з квадратом
	int   GridMeters        = 1000;  // точність квадрата в оголошенні, м
	float LastHitSeconds    = 60;    // смерть від кровотечі / виходу в бою зараховується тому, хто влучив останнім
	float NotifySeconds     = 10;    // скільки висить повідомлення
	ref array<string> RewardItems = {}; // що отримує той, хто зняв голову (класи предметів)
}

// Тільки серверні, клієнту не передаються.
class OblivionCombatLogSettings
{
	bool  Enabled            = true;
	float CombatSeconds      = 60;   // скільки триває «бій» після останнього влучання гравець↔гравець
	bool  KillOnLeave        = true; // вийшов у бою -> персонаж помирає (тіло з лутом лишається)
	int   StayInWorldSeconds = 60;   // якщо KillOnLeave = false: скільки персонаж стоїть у світі після виходу
	bool  Notify             = true; // повідомлення гравцю при вході в бій
}

// Серверні налаштування, файл <профіль сервера>/Oblivion/settings.json.
// Клієнт отримує копію при підключенні (OBLIVION_RPC_SETTINGS), щоб умови дій збігались із сервером.
// Кожна нова механіка додає сюди свій блок.
class OblivionSettings
{
	int ConfigVersion = 1;
	ref OblivionRadiatorRepairSettings RadiatorRepair = new OblivionRadiatorRepairSettings();
	ref OblivionMetalPlatesSettings    MetalPlates    = new OblivionMetalPlatesSettings();
	ref OblivionVehicleActionsSettings VehicleActions = new OblivionVehicleActionsSettings();
	ref OblivionCombatLogSettings      CombatLog      = new OblivionCombatLogSettings();
	ref OblivionBountySettings         Bounty         = new OblivionBountySettings();

	private static ref OblivionSettings s_Instance;
	static int s_Revision; // росте при кожній заміні налаштувань — для кешів

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
			JsonFileLoader<OblivionSettings>.JsonLoadFile(OBLIVION_SETTINGS_FILE, settings);

		if (!settings.RadiatorRepair)
			settings.RadiatorRepair = new OblivionRadiatorRepairSettings();
		if (!settings.MetalPlates)
			settings.MetalPlates = new OblivionMetalPlatesSettings();
		if (!settings.VehicleActions)
			settings.VehicleActions = new OblivionVehicleActionsSettings();
		if (!settings.VehicleActions.AllowedActions)
			settings.VehicleActions.AllowedActions = new array<string>();
		if (!settings.CombatLog)
			settings.CombatLog = new OblivionCombatLogSettings();
		if (!settings.Bounty)
			settings.Bounty = new OblivionBountySettings();
		if (!settings.Bounty.RewardItems)
			settings.Bounty.RewardItems = new array<string>();

		// Перезаписуємо, щоб нові поля з'явились у файлі з дефолтними значеннями.
		JsonFileLoader<OblivionSettings>.JsonSaveFile(OBLIVION_SETTINGS_FILE, settings);
		return settings;
	}

	void WriteSync(ParamsWriteContext ctx)
	{
		RadiatorRepair.Write(ctx);
		MetalPlates.Write(ctx);
		VehicleActions.Write(ctx);
	}

	static void ReadSync(ParamsReadContext ctx)
	{
		OblivionSettings settings = new OblivionSettings();
		if (settings.RadiatorRepair.Read(ctx) && settings.MetalPlates.Read(ctx) && settings.VehicleActions.Read(ctx))
		{
			s_Instance = settings;
			s_Revision++;
		}
	}
}
