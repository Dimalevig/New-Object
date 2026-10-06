modded class PlayerBase {
	
	static ref array<PlayerBase> bxd_player_list = new array<PlayerBase>();
	int identityIdHash = 0;
	
	void PlayerBase() {
		if (GetGame() && GetGame().IsClient()) {
			bxd_player_list.Insert(this);
		}
		RegisterNetSyncVariableInt("identityIdHash");
	}
	
	void ~PlayerBase() {
		if (GetGame() && GetGame().IsClient()) {
			bxd_player_list.RemoveItem(this);
		}
	}

	ref OBLParty bxdgroup;
	ref OBLPartyMember memberCache;
	
	OBLParty GetOBLParty() {
		return bxdgroup;
	}
	
	OBLPartyPermission GetPermission() {
		OBLPartyMember member = GetMyGroupMarker();
		if (!member)
			return null;
		return OBLPartyPermissions.Get().FindPermissionGroupByUID(member.permissionGroup);
	}
	
	OBLPartyMember GetMyGroupMarker() {
		if (!GetOBLParty())
			return null;
		if (memberCache)
			return memberCache;
		string steamid = GetMySteamId();
		OBLPartyMember member = GetOBLParty().GetMemberBySteamid(steamid);
		memberCache = member;
		return member;
	}
	
	static PlayerBase GetPlayerByIdentity(PlayerIdentity ident) {
		if (!ident)
			return null;
		int low, high;
		int id = ident.GetPlayerId();
		GetGame().GetPlayerNetworkIDByIdentityID(id, low, high);
		Object obj = GetGame().GetObjectByNetworkId(low, high);
		return PlayerBase.Cast(obj);
	}
	
	int GetMySubGroup() {
		OBLPartyMember memb = GetMyGroupMarker();
		if (memb)
			return memb.currentSubgroup;
		return -1;
	}
	
	string GetMySteamId() {
		if (GetGame().IsServer()) {
			if (!GetIdentity())
				return "";
			return GetIdentity().GetPlainId();
		} else {
			MissionBaseWorld mission = MissionBaseWorld.Cast(GetGame().GetMission());
			if (!mission)
				return "";
			return mission.mySteamid;
		}
	}
	
	void SetOBLParty(OBLParty grp) {
		if (GetGame().IsClient()) {
			if (bxdgroup)
				delete bxdgroup;
		}
		bxdgroup = grp;
		OnGroupChanged();
	}
	
	void OnGroupChanged() {
		memberCache = null;
		MissionBaseWorld mission = MissionBaseWorld.Cast(GetGame().GetMission());
		if (mission)
			mission.OnGroupChanged();
	}

	void AddSimpleClientMarker(string name, string icon, vector position, int color, string creatorId = "Server") {
		if (!g_Game.IsServer() || !GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(name);
		rpc.Write(icon);
		rpc.Write(position);
		rpc.Write(color);
		rpc.Write(creatorId);
		rpc.Send(null, OBLPartyRPCs.GROUP_ADD_CLIENT_MARKER, true, GetIdentity());
	}
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx) {
		super.OnRPC(sender, rpc_type, ctx);
		if (rpc_type == OBLPartyRPCs.GROUP_SYNC) {
			bool has = false;
			if (!ctx.Read(has))
				return;
			if (!has) {
				SetOBLParty(null);
				return;
			}
			OBLParty grp = new OBLParty();
			if (!grp.ReadFromCtx(ctx)) {
				OBLLogger.Debug("Failed to receive Group from Server !");
				return;
			}
			OBLLogger.Debug("Successfully received Group from Server");
			SetOBLParty(grp);
			grp.InitMarkers();
		} else if (rpc_type == OBLPartyRPCs.GROUP_INVITE) {
			Param1<string> shortnameParam;
			if (!ctx.Read(shortnameParam)) {
				OBLLogger.Debug("Failed to receive Shortname of Group Invite !");
				return;
			}
			MissionBaseWorld mission = MissionBaseWorld.Cast(GetGame().GetMission());
			if (!mission)
				return;
			mission.lastInvite = shortnameParam.param1;
			// OBL FIX: empty shortname = server expired the invite, forget it silently
			if (mission.lastInvite == "") {
				OBLLogger.Debug("Group invite expired / cleared by server");
				return;
			}
			OBLLogger.Debug("Recevied Invite to Group " + mission.lastInvite);
		}
	}
	
	override void SetActionsRemoteTarget( out TInputActionMap InputActionMap) {
		super.SetActionsRemoteTarget(InputActionMap);
		AddAction(ActionInvitePlayerToGroup, InputActionMap);
	}

	override void EEKilled(Object killer) {
		AddSimpleClientMarker("Місце смерті", "OBL_SystemParty\\gui\\icons\\skull.paa", GetPosition(), ARGB(220, 224, 70, 90), "PM" );
		super.EEKilled(killer);		
	}
}
