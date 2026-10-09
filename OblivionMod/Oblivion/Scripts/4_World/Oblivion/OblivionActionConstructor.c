modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionRepairCarRadiatorEpoxy);
		actions.Insert(ActionMineCar);
		actions.Insert(ActionInspectCarEngine);
		actions.Insert(ActionDefuseCar);
	}
}
