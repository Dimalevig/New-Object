// Ці машини у ванільному CanDisplayCargo ховають вантаж при закритому багажнику.
// Для того, хто сидить усередині, показуємо вантаж завжди.
modded class OffroadHatchback
{
	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}
}

modded class CivilianSedan
{
	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}
}

modded class Hatchback_02
{
	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}
}

modded class Sedan_02
{
	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}
}

modded class Offroad_02
{
	override bool CanDisplayCargo()
	{
		if (OblivionCargoFromInside())
			return true;
		return super.CanDisplayCargo();
	}
}
