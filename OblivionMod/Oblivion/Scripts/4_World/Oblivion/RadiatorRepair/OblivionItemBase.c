modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();

		// Перевірка за конфіг-класом, щоб працювало незалежно від того, чи є у ванілі скрипт-клас EpoxyPutty.
		if (IsKindOf("EpoxyPutty"))
			AddAction(ActionRepairCarRadiatorEpoxy);

		if (IsKindOf("Grenade_Base"))
			AddAction(ActionMineCar);
	}
}
