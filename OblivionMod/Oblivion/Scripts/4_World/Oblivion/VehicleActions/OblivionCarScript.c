// Доступ до інвентаря машини для тих, хто сидить усередині: у русі й при закритому багажнику.
modded class CarScript
{
	static CarScript OblivionGetVehicleOf(PlayerBase player)
	{
		if (!player)
			return null;

		HumanCommandVehicle hcv = player.GetCommand_Vehicle();
		if (!hcv)
			return null;

		return CarScript.Cast(hcv.GetTransport());
	}

	protected bool OblivionCargoFromInside()
	{
		OblivionVehicleActionsSettings s = OblivionSettings.Get().VehicleActions;
		if (!s.Enabled || !s.CargoFromInside)
			return false;

		// Клієнт: чи сидить у цій машині сам гравець.
		if (GetGame().IsClient())
			return OblivionGetVehicleOf(PlayerBase.Cast(GetGame().GetPlayer())) == this;

		// Сервер: чи є в машині хоч хтось.
		for (int i = 0; i < CrewSize(); i++)
		{
			if (CrewMember(i))
				return true;
		}
		return false;
	}

	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanReleaseCargo(cargo);
	}
}
