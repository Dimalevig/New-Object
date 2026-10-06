modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();

		// Checked by config class so it works whether or not vanilla has a script class for EpoxyPutty.
		if (IsKindOf("EpoxyPutty"))
			AddAction(ActionRepairCarRadiatorEpoxy);
	}
}
