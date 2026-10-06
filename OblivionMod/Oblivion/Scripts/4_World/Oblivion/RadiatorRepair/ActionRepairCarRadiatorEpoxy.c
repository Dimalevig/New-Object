class ActionRepairCarRadiatorEpoxyCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(OblivionSettings.Get().RadiatorRepair.DurationSeconds);
	}
}

// Епоксидка в руках -> дивимось на машину (радіатор встановлений) або на радіатор на землі.
class ActionRepairCarRadiatorEpoxy : ActionContinuousBase
{
	void ActionRepairCarRadiatorEpoxy()
	{
		m_CallbackClass    = ActionRepairCarRadiatorEpoxyCB;
		m_CommandUID       = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody         = true;
		m_StanceMask       = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight  = UASoftSkillsWeight.PRECISE_LOW;
		m_Text             = "#STR_OBLIVION_REPAIR_RADIATOR";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINonRuined;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}

	static ItemBase FindRadiator(ActionTarget target, out CarScript car)
	{
		car = null;
		if (!target)
			return null;

		Object obj = target.GetObject();

		ItemBase item = ItemBase.Cast(obj);
		if (item && item.IsKindOf("CarRadiator"))
		{
			car = CarScript.Cast(item.GetHierarchyParent());
			return item;
		}

		car = CarScript.Cast(obj);
		if (!car)
			car = CarScript.Cast(target.GetParent());
		if (!car)
			return null;

		return ItemBase.Cast(car.FindAttachmentBySlotName("CarRadiator"));
	}

	static bool CanRepair(ItemBase radiator, CarScript car)
	{
		OblivionRadiatorRepairSettings s = OblivionSettings.Get().RadiatorRepair;

		if (!s.Enabled || !radiator)
			return false;

		if (radiator.IsRuined() && !s.AllowRuined)
			return false;

		if (radiator.GetHealth01("", "") >= s.RepairToHealthPercent / 100)
			return false;

		// Радіатор у машині: те саме правило доступу, що й для зняття (відкритий капот тощо).
		if (car && s.RequireAccessible && !car.CanReleaseAttachment(radiator))
			return false;

		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car;
		ItemBase radiator = FindRadiator(target, car);
		return CanRepair(radiator, car);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car;
		ItemBase radiator = FindRadiator(action_data.m_Target, car);
		if (!CanRepair(radiator, car))
			return;

		OblivionRadiatorRepairSettings s = OblivionSettings.Get().RadiatorRepair;

		radiator.SetHealth("", "", radiator.GetMaxHealth("", "") * s.RepairToHealthPercent / 100);
		ConsumeEpoxy(action_data.m_MainItem, s.RepairsPerEpoxy);
	}

	protected void ConsumeEpoxy(ItemBase epoxy, int repairsPerItem)
	{
		if (!epoxy)
			return;

		int uses = Math.Max(1, repairsPerItem);

		if (epoxy.HasQuantity())
			epoxy.AddQuantity(-Math.Ceil(epoxy.GetQuantityMax() / uses));
		else
			epoxy.AddHealth("", "", -epoxy.GetMaxHealth("", "") / uses);
	}
}
