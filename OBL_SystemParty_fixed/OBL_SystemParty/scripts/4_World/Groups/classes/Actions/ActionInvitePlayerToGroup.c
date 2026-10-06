class ActionInvitePlayerToGroup : ActionInteractBase {
	
	void ActionInvitePlayerToGroup()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}
	
	override void CreateConditionComponents()  
	{		
		m_ConditionTarget = new CCTMan(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINone;
	}
	
	override string GetText()
	{
		return "Запросити гравця";
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item ) {
		OBLParty grp = player.GetOBLParty();
		if (!grp)
			return false;
		OBLPartyPermission perms = player.GetPermission();
		if (!perms)
			return false;
		PlayerBase targetPlayer = PlayerBase.Cast(target.GetObject());
		return perms.canInvite && targetPlayer && targetPlayer.GetIdentity();
	}
	
	override void OnExecuteServer( ActionData action_data ) {
		// OBL FIX: the action did nothing; the invite logic lives in the server mod (OBLParty.OnInviteActionServer)
		if (!action_data || !action_data.m_Player || !action_data.m_Target)
			return;
		PlayerBase targetPlayer = PlayerBase.Cast(action_data.m_Target.GetObject());
		OBLParty grp = action_data.m_Player.GetOBLParty();
		if (grp && targetPlayer)
			grp.OnInviteActionServer(action_data.m_Player, targetPlayer);
	}
}
