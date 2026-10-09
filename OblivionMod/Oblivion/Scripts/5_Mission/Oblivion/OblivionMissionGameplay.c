// Швидкі слоти (1-9) у машині: рушій не дає взяти річ у руки, тож клієнт лише каже серверу,
// яку річ зі слота взяти, а перекладає її сервер. Поза машиною — нічого не робить.
modded class MissionGameplay
{
	protected ref array<bool> m_OblivionSlotKeysDown;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (!player || !player.IsInVehicle())
			return;

		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		if (!s.Enabled || !s.QuickbarInVehicle)
			return;

		if (!m_OblivionSlotKeysDown)
		{
			m_OblivionSlotKeysDown = new array<bool>();
			for (int k = 0; k < 9; k++)
				m_OblivionSlotKeysDown.Insert(false);
		}

		bool menuOpen = GetUIManager().GetMenu() != null;
		array<int> keys = {KeyCode.KC_1, KeyCode.KC_2, KeyCode.KC_3, KeyCode.KC_4, KeyCode.KC_5, KeyCode.KC_6, KeyCode.KC_7, KeyCode.KC_8, KeyCode.KC_9};

		for (int i = 0; i < 9; i++)
		{
			bool down = KeyState(keys[i]) != 0;
			bool pressed = down && !m_OblivionSlotKeysDown[i];
			m_OblivionSlotKeysDown[i] = down;

			if (!pressed || menuOpen)
				continue;

			EntityAI item = player.GetQuickBarEntity(i);
			if (!item)
				continue;

			int low, high;
			item.GetNetworkID(low, high);
			GetGame().RPCSingleParam(player, OBLIVION_RPC_QUICKBAR, new Param2<int, int>(low, high), true);
		}
	}
}
