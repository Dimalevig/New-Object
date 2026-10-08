// Дозволяє дії зі списку settings.json -> VehicleActions, коли гравець сидить у машині.
// Результат перевірки кешується, тож список не перебирається щокадру.
modded class ActionBase
{
	protected bool m_OblivionInVehicle;
	protected int  m_OblivionCacheRev = -1;

	override bool CanBeUsedInVehicle()
	{
		if (super.CanBeUsedInVehicle())
			return true;

		OblivionSettings settings = OblivionSettings.Get();
		if (m_OblivionCacheRev != OblivionSettings.s_Revision)
		{
			m_OblivionInVehicle = OblivionIsAllowedInVehicle(settings.VehicleActions);
			m_OblivionCacheRev  = OblivionSettings.s_Revision;
		}
		return m_OblivionInVehicle;
	}

	protected bool OblivionIsAllowedInVehicle(OblivionVehicleActionsSettings s)
	{
		if (!s.Enabled)
			return false;

		typename t = Type();
		while (t)
		{
			if (s.AllowedActions.Find(t.ToString()) != -1)
				return true;
			t = t.Parent();
		}
		return false;
	}
}
