// Позначка «в бою»: ставиться тільки при влученні гравця в гравця, без таймерів.
modded class PlayerBase
{
	protected int m_OblivionCombatUntil; // час GetGame().GetTime(), мс

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!GetGame().IsServer() || !source)
			return;

		OblivionCombatLogSettings s = OblivionSettings.Get().CombatLog;
		if (!s.Enabled)
			return;

		PlayerBase attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());
		if (!attacker || attacker == this)
			return;

		OblivionMarkInCombat(s);
		attacker.OblivionMarkInCombat(s);
	}

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
