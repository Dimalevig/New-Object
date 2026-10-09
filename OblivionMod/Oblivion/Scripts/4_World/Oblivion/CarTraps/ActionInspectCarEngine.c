class ActionInspectCarEngineCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(OblivionSettings.Get().CarTraps.InspectSeconds);
	}
}

// Порожні руки, капот відкритий -> «Оглянути двигун»: сервер каже, чи є під капотом граната.
class ActionInspectCarEngine : ActionContinuousBase
{
	void ActionInspectCarEngine()
	{
		m_CallbackClass = ActionInspectCarEngineCB;
		m_CommandUID    = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody      = true;
		m_StanceMask    = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text          = "#STR_OBLIVION_INSPECT_ENGINE";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINone;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}

	override bool HasTarget()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!OblivionSettings.Get().CarTraps.Enabled)
			return false;

		CarScript car = ActionMineCar.GetCar(target);
		return car && !car.IsRuined() && !car.EngineIsOn() && car.OblivionIsHoodOpen() && car.OblivionIsNearEngine(player);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car = ActionMineCar.GetCar(action_data.m_Target);
		if (!car)
			return;

		if (car.OblivionIsMined())
			OblivionNotify.ToPlayer(action_data.m_Player, "Під капотом граната!", "Візьми пасатижі й розмінуй машину.", 8);
		else
			OblivionNotify.ToPlayer(action_data.m_Player, "Огляд двигуна", "Нічого підозрілого.", 5);
	}
}
