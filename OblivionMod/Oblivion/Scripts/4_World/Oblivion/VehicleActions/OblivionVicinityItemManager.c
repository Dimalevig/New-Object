// Показує машину, в якій сидить гравець, у вікні «поруч» інвентаря.
modded class VicinityItemManager
{
	override void RefreshVicinityItems()
	{
		super.RefreshVicinityItems();

		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		if (!s.Enabled || !s.CargoFromInside)
			return;

		CarScript car = CarScript.OblivionGetVehicleOf(PlayerBase.Cast(GetGame().GetPlayer()));
		if (!car)
			return;

		if (GetVicinityItems().Find(car) == -1)
			AddVicinityItems(car);
	}
}
