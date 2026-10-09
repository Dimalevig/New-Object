class ActionMineCarCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(OblivionSettings.Get().CarTraps.DurationSeconds);
	}
}

// Граната в руках -> дивимось на машину з заглушеним двигуном -> граната під капотом.
// Стан «замінована» клієнту не передається, щоб міну не можна було виявити.
class ActionMineCar : ActionContinuousBase
{
	void ActionMineCar()
	{
		m_CallbackClass   = ActionMineCarCB;
		m_CommandUID      = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody        = true;
		m_StanceMask      = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
		m_Text            = "#STR_OBLIVION_MINE_CAR";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINonRuined;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}

	static CarScript GetCar(ActionTarget target)
	{
		if (!target)
			return null;

		CarScript car = CarScript.Cast(target.GetObject());
		if (!car)
			car = CarScript.Cast(target.GetParent());
		return car;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		OblivionCarTrapsSettings s = OblivionSettings.Get().CarTraps;
		if (!s.Enabled || !item || s.Grenades.Find(item.GetType()) == -1)
			return false;

		CarScript car = GetCar(target);
		return car && !car.IsRuined() && !car.EngineIsOn();
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car = GetCar(action_data.m_Target);
		ItemBase grenade = action_data.m_MainItem;
		if (!car || !grenade || car.OblivionIsMined())
			return;

		car.OblivionSetMine(grenade.GetType());
		grenade.DeleteSafe();
	}
}
