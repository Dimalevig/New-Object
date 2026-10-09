// Позначка «в бою»: ставиться при влученні гравця в гравця (див. OblivionPlayerBase.EEHitBy), без таймерів.
modded class PlayerBase
{
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
