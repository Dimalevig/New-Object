class ActionDefuseCarCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(OblivionSettings.Get().CarTraps.DefuseSeconds);
	}
}

// Пасатижі в руках, капот відкритий -> «Розмінувати»: граната повертається гравцю. Без шансу провалу.
class ActionDefuseCar : ActionContinuousBase
{
	void ActionDefuseCar()
	{
		m_CallbackClass   = ActionDefuseCarCB;
		m_CommandUID      = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody        = true;
		m_StanceMask      = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
		m_Text            = "#STR_OBLIVION_DEFUSE_CAR";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINonRuined;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		OblivionCarTrapsSettings s = OblivionSettings.Get().CarTraps;
		if (!s.Enabled || !item || !item.IsKindOf(s.DefuseTool))
			return false;

		CarScript car = ActionMineCar.GetCar(target);
		return car && !car.IsRuined() && !car.EngineIsOn() && car.OblivionIsHoodOpen();
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car = ActionMineCar.GetCar(action_data.m_Target);
		if (!car)
			return;

		string grenade = car.OblivionTakeMine();
		if (grenade == "")
		{
			OblivionNotify.ToPlayer(action_data.m_Player, "Розмінування", "Міни немає.", 5);
			return;
		}

		OblivionNotify.SpawnItem(action_data.m_Player, grenade, action_data.m_Player.GetPosition());
		OblivionNotify.ToPlayer(action_data.m_Player, "Розміновано", "Граната у тебе в інвентарі (або під ногами).", 6);
	}
}
