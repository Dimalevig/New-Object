modded class PlayerBase
{
	protected string     m_OblivionUid;          // Steam ID, кешується, бо після виходу identity вже немає
	protected PlayerBase m_OblivionLastAttacker;
	protected int        m_OblivionLastHitTime;  // GetGame().GetTime(), мс

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == OBLIVION_RPC_SETTINGS && GetGame().IsClient())
			OblivionSettings.ReadSync(ctx);
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

		if (GetGame().IsServer())
			OblivionBounty.OnPlayerKilled(this, OblivionFindKiller(killer));
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
