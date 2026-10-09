// Машина, в якій сидить гравець, — у вікні «поруч» (щоб був видно багажник).
modded class VicinityItemManager
{
	override void RefreshVicinityItems()
	{
		super.RefreshVicinityItems();

		CarScript car = CarScript.OblivionGetVehicleOf(PlayerBase.Cast(GetGame().GetPlayer()));
		if (car && car.OblivionCargoFromInside())
			AddVicinityItems(car); // сам відкидає дублікати
	}
}
