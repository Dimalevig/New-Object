// Інвентар у машині: гра всередині машини не відкриває вікно інвентаря (CanOpenInventory = false).
// Для гравця в машині відкриваємо його самі, решта — як у ванілі.
modded class MissionGameplay
{
	protected bool m_OblivionTabWasDown;

	protected bool OblivionInventoryInVehicle()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsInVehicle())
			return false;

		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		return s.Enabled && s.CargoFromInside;
	}

	override void ShowInventory()
	{
		if (!OblivionInventoryInVehicle())
		{
			super.ShowInventory();
			return;
		}

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (GetUIManager().GetMenu() || player.IsInventorySoftLocked())
			return;

		if (!m_InventoryMenu)
			InitInventory();

		if (!GetUIManager().FindMenu(MENU_INVENTORY))
		{
			GetUIManager().ShowScriptedMenu(m_InventoryMenu, null);
			player.OnInventoryMenuOpen();
		}
		AddActiveInputExcludes({"inventory"});
		AddActiveInputRestriction(EInputRestrictors.INVENTORY);
	}

	// Запасний варіант: якщо в машині гра не передає натискання «Інвентар», ловимо Tab напряму.
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		bool tabDown = KeyState(KeyCode.KC_TAB) != 0;
		bool tabPressed = tabDown && !m_OblivionTabWasDown;
		m_OblivionTabWasDown = tabDown;

		if (!tabPressed || !OblivionInventoryInVehicle())
			return;

		// Ваніль уже обробила натискання — не дублюємо.
		if (GetUApi().GetInputByID(UAGear).LocalPress())
			return;

		UIScriptedMenu menu = GetUIManager().GetMenu();
		if (!menu)
			ShowInventory();
		else if (menu == m_InventoryMenu)
			HideInventory();
	}
}
