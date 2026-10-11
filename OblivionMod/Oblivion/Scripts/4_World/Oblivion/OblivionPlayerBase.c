// Усі зміни PlayerBase — в одному файлі: різні modded-шари не бачать методів один одного.
modded class PlayerBase
{
	protected string     m_OblivionUid;          // Steam ID, кешується, бо після виходу identity вже немає
	protected PlayerBase m_OblivionLastAttacker;
	protected int        m_OblivionLastHitTime;  // GetGame().GetTime(), мс
	protected int        m_OblivionSpawnTime;
	protected bool       m_OblivionLoadedFromDb; // персонаж завантажений з бази = не новий

	// Дії з порожніми руками.
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(ActionInspectCarEngine, InputActionMap);
	}

	override void EEInit()
	{
		super.EEInit();
		m_OblivionSpawnTime = GetGame().GetTime();
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		m_OblivionLoadedFromDb = true;
		return super.OnStoreLoad(ctx, version);
	}

	// Новачок: персонаж створений у цій сесії сервера менше N хв тому.
	bool OblivionIsFreshSpawn(float minutes)
	{
		return !m_OblivionLoadedFromDb && GetGame().GetTime() - m_OblivionSpawnTime < minutes * 60000;
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == OBLIVION_RPC_SETTINGS && GetGame().IsClient())
			OblivionSettings.ReadSync(ctx);

		if (rpc_type == OBLIVION_RPC_COMMAND && GetGame().IsServer())
			OblivionOnChatCommand(sender, ctx);
	}

	// --- Інвентар у машині. Ваніль при посадці замикає інвентар скриптовим замком
	// і забороняє брати речі в руки; знімаємо обидва обмеження (якщо ввімкнено).

	protected bool OblivionInventoryInVehicle()
	{
		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		return s.Enabled && s.InventoryInVehicle;
	}

	protected bool m_OblivionUnlockedInVehicle; // ми зняли ванільний замок при посадці

	override void OnCommandVehicleStart()
	{
		super.OnCommandVehicleStart();

		if (OblivionInventoryInVehicle() && GetInventory() && !m_OblivionUnlockedInVehicle)
		{
			GetInventory().UnlockInventory(LOCK_FROM_SCRIPT);
			m_OblivionUnlockedInVehicle = true;
		}
	}

	override void OnCommandVehicleFinish()
	{
		// Ванільний OnCommandVehicleFinish знімає замок — повертаємо той, що зняли при посадці, щоб лічильник зійшовся.
		if (m_OblivionUnlockedInVehicle && GetInventory())
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		m_OblivionUnlockedInVehicle = false;

		super.OnCommandVehicleFinish();

		// Страховка: після виходу ванільні замки (дія «Вийти», падіння) мають зникнути;
		// якщо щось лишилося — інвентар залишився б замкненим до перезаходу.
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OblivionReleaseStaleInventoryLock, 1500, false);
	}

	protected void OblivionReleaseStaleInventoryLock()
	{
		GameInventory inventory = GetInventory();
		if (!inventory || !IsAlive())
			return;

		// Ці стани ваніль замикає законно — не чіпаємо.
		if (IsInVehicle() || GetCommand_Vehicle() || GetCommand_Fall() || GetCommand_Swim() || GetCommand_Ladder() || GetCommand_Climb())
			return;

		int guard = 0;
		while (inventory.IsInventoryLockedForLockType(LOCK_FROM_SCRIPT) && guard < 10)
		{
			inventory.UnlockInventory(LOCK_FROM_SCRIPT);
			guard++;
		}
	}

	override bool CanReceiveItemIntoHands(EntityAI item_to_hands)
	{
		if (IsInVehicle() && OblivionInventoryInVehicle())
			return CanPickupHeavyItem(item_to_hands);

		return super.CanReceiveItemIntoHands(item_to_hands);
	}

	// Команда з чату; приймаємо лише від самого гравця.
	protected void OblivionOnChatCommand(PlayerIdentity sender, ParamsReadContext ctx)
	{
		if (!sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
			return;

		Param1<string> data = new Param1<string>("");
		if (ctx.Read(data))
			OblivionRunChatCommand(data.param1);
	}

	// Виконати команду мода від імені цього гравця (сервер).
	bool OblivionRunChatCommand(string text)
	{
		if (!OblivionIsChatCommand(text))
			return false;

		array<string> words;
		OblivionSplitCommand(text, words);

		string cmd = OblivionNormalizeCommand(words[0]);
		if (cmd == "/bounty" || cmd == "/баунті")
			OblivionBounty.HandleCommand(this);
		else if (cmd == "/reward" || cmd == "/нагорода")
			OblivionPlaytimeRewards.HandleCommand(this);
		else
			OblivionContracts.HandleCommand(this, words);
		return true;
	}

	string OblivionGetUid()
	{
		if (GetIdentity())
			m_OblivionUid = GetIdentity().GetPlainId();
		return m_OblivionUid;
	}

	string OblivionGetName()
	{
		if (GetIdentity())
			return GetIdentity().GetName();
		return "?";
	}

	// Влучання гравця в гравця: бій (combat log) + хто влучив останнім (баунті).
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!GetGame().IsServer() || !source)
			return;

		PlayerBase attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());
		if (!attacker || attacker == this)
			return;

		m_OblivionLastAttacker = attacker;
		m_OblivionLastHitTime  = GetGame().GetTime();
		OblivionGetUid();
		attacker.OblivionGetUid();

		OblivionCombatLogSettings combat = OblivionSettings.Get().CombatLog;
		if (combat.Enabled)
		{
			OblivionMarkInCombat(combat);
			attacker.OblivionMarkInCombat(combat);
		}
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		PlayerBase killerPlayer = OblivionFindKiller(killer);
		OblivionBounty.OnPlayerKilled(this, killerPlayer);
		OblivionContracts.OnPlayerKilled(this, killerPlayer);
		OblivionKillEvents.OnPlayerKilled(this, killerPlayer);
	}

	protected PlayerBase OblivionFindKiller(Object killer)
	{
		PlayerBase result;
		EntityAI killerEntity = EntityAI.Cast(killer);
		if (killerEntity)
			result = PlayerBase.Cast(killerEntity.GetHierarchyRootPlayer());

		// Кровотеча, вихід у бою тощо: зараховуємо тому, хто влучив останнім.
		if (!result || result == this)
		{
			float window = OblivionSettings.Get().Bounty.LastHitSeconds * 1000;
			if (m_OblivionLastAttacker && GetGame().GetTime() - m_OblivionLastHitTime <= window)
				result = m_OblivionLastAttacker;
		}

		if (result == this)
			return null;
		return result;
	}

	// --- Combat log: позначка «в бою», ставиться при влученні гравця в гравця (див. EEHitBy), без таймерів.

	protected int m_OblivionCombatUntil; // час GetGame().GetTime(), мс

	void OblivionMarkInCombat(OblivionCombatLogSettings s)
	{
		bool wasInCombat = OblivionIsInCombat();
		m_OblivionCombatUntil = GetGame().GetTime() + s.CombatSeconds * 1000;

		if (!wasInCombat && s.Notify && GetIdentity())
		{
			string text = "Вихід з гри протягом " + s.CombatSeconds + " с після бою — ";
			if (s.KillOnLeave)
				text += "смерть персонажа.";
			else
				text += "персонаж лишиться у світі на " + s.StayInWorldSeconds + " с.";

			NotificationSystem.SendNotificationToPlayerIdentityExtended(GetIdentity(), 5, "Ти в бою", text);
		}
	}

	bool OblivionIsInCombat()
	{
		return m_OblivionCombatUntil > GetGame().GetTime();
	}
}
