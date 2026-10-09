modded class PlayerBase
{
	protected string     m_OblivionUid;          // Steam ID, кешується, бо після виходу identity вже немає
	protected PlayerBase m_OblivionLastAttacker;
	protected int        m_OblivionLastHitTime;  // GetGame().GetTime(), мс
	protected int        m_OblivionSpawnTime;
	protected bool       m_OblivionLoadedFromDb; // персонаж завантажений з бази = не новий

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

	// Команда з чату; приймаємо лише від самого гравця.
	protected void OblivionOnChatCommand(PlayerIdentity sender, ParamsReadContext ctx)
	{
		if (!sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
			return;

		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data) || !OblivionIsChatCommand(data.param1))
			return;

		array<string> words;
		OblivionSplitCommand(data.param1, words);

		string cmd = words[0];
		cmd.ToLower();
		if (cmd == "/bounty" || cmd == "/баунті")
			OblivionBounty.HandleCommand(this);
		else if (cmd == "/reward" || cmd == "/нагорода")
			OblivionPlaytimeRewards.HandleCommand(this);
		else
			OblivionContracts.HandleCommand(this, words);
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
}
