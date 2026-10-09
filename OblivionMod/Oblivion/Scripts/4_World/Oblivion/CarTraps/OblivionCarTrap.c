// Граната під капотом вибухає при запуску двигуна. Діє до рестарту (не зберігається в базу).
modded class CarScript
{
	protected string m_OblivionMine; // клас гранати, "" = не замінована

	bool OblivionIsMined()
	{
		return m_OblivionMine != "";
	}

	void OblivionSetMine(string grenadeType)
	{
		m_OblivionMine = grenadeType;
	}

	override void OnEngineStart()
	{
		super.OnEngineStart();

		if (!GetGame().IsServer() || m_OblivionMine == "")
			return;

		string type = m_OblivionMine;
		m_OblivionMine = "";

		EntityAI grenade = EntityAI.Cast(GetGame().CreateObjectEx(type, GetPosition() + "0 0.5 0", ECE_NONE));
		if (!grenade)
			return;

		grenade.Explode(DamageType.EXPLOSION);
		// Видаляємо трохи пізніше, щоб клієнти встигли отримати ефект вибуху.
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().ObjectDelete, 1000, false, grenade);
	}
}
