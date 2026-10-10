// Дії зі списку settings.json -> VehicleActions дозволені в машині.
// Кешується: список перебирається один раз на дію (і заново, коли сервер надіслав нові налаштування).
modded class ActionBase
{
	protected bool m_OblivionListed;
	protected int  m_OblivionCacheRev = -1;

	// Дія є в нашому списку (сама або через батьківський клас).
	bool OblivionIsListedForVehicle()
	{
		if (m_OblivionCacheRev != OblivionSettings.s_Revision)
		{
			m_OblivionListed   = OblivionComputeListed(OblivionSettings.Get().VehicleActions);
			m_OblivionCacheRev = OblivionSettings.s_Revision;
		}
		return m_OblivionListed;
	}

	protected bool OblivionComputeListed(OblivionVehicleActionsSettings s)
	{
		if (!s.Enabled)
			return false;

		typename own = Type();
		foreach (string name : s.AllowedActions)
		{
			typename allowed = name.ToType();
			if (allowed && (own == allowed || own.IsInherited(allowed)))
				return true;
		}
		return false;
	}

	// Гравець у машині і дія — з нашого списку.
	bool OblivionIsVehicleMode(PlayerBase player)
	{
		return player && player.IsInVehicle() && OblivionIsListedForVehicle();
	}

	override bool CanBeUsedInVehicle()
	{
		if (super.CanBeUsedInVehicle())
			return true;
		return OblivionIsListedForVehicle();
	}

	// У машині немає пози «стоячи/навпочіпки», тож ваніль вибирає анімацію на все тіло,
	// яку в машині запустити не можна. Для наших дій — лише верхня частина тіла і будь-яка поза.
	override protected bool IsFullBodyEx(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (OblivionIsVehicleMode(player))
			return false;
		return super.IsFullBodyEx(player, target, item);
	}

	override protected int GetStanceMaskEx(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (OblivionIsVehicleMode(player))
			return DayZPlayerConstants.STANCEMASK_ALL;
		return super.GetStanceMaskEx(player, target, item);
	}
}

modded class AnimatedActionBase
{
	// Анімація для верхньої частини тіла в машині: у дій з «прон-винятком» це m_CommandUID,
	// дії лише з анімацією на все тіло (бинт, шина, укол) отримують загальну «роботу руками».
	override protected int GetActionCommand(PlayerBase player)
	{
		if (OblivionIsVehicleMode(player))
		{
			if (HasProneException())
				return m_CommandUID;
			return DayZPlayerConstants.CMD_ACTIONMOD_CRAFTING;
		}
		return super.GetActionCommand(player);
	}
}
