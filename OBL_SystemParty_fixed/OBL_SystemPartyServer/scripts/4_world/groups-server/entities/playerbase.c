modded class PlayerBase {
	
	string steamid;
	
	void InitGroupServer(PlayerIdentity identity, OBLParty lgroup = null) {
		OBLLogger.Debug("InitGroupServer");
		if (identity)
			steamid = identity.GetPlainId();
		if (!identity && steamid == "") {
			ClearGroupServer();
			return;
		}
		if (!lgroup)
			bxdgroup = OBLPartyManager.Get().GetPlayersGroup(steamid);
		else
			bxdgroup = lgroup;
		if (bxdgroup) {
			bxdgroup.OnPlayerOnline(this, identity);
		}
		OBLPartyMember member = GetMyGroupMarker();
		if (member) {
			member.SetHealth(GetHealth());
			if (GetIdentity()) {
				member.SetName(GetIdentity().GetName());
				member.SetOnline(true, GetIdentity().GetId());
			}
		}
		SendGroupInfo();
		OBLLogger.Debug("Finish InitGroupServer");
	}
	
	override void SetOBLParty(OBLParty grp) {
		if (!grp) {
			ClearGroupServer();
			return;
		}
		super.SetOBLParty(grp);
		InitGroupServer(GetIdentity(), grp);
	}
	
	void ClearGroupServer(OBLParty expectedGroup = null) {
		if (expectedGroup && bxdgroup != expectedGroup)
			return;
		bxdgroup = null;
		memberCache = null;
		SendGroupInfo();
	}
	
	void InitGroupServerRespawn(PlayerIdentity identity) {
		steamid = identity.GetPlainId();
		bxdgroup = OBLPartyManager.Get().GetPlayersGroup(steamid);
		if (bxdgroup) {
			bxdgroup.OnPlayerRespawn(this, identity);
			OBLPartyMember member = GetMyGroupMarker();
			if (member) {
				member.SetHealth(GetHealth());
				member.SetOnline(true, identity.GetId());
			}
		}
	}
	
	override void OnDisconnect() {
		super.OnDisconnect();
		if (bxdgroup) {
			bxdgroup.OnPlayerOffline(this, steamid);
			OBLPartyMember member = GetMyGroupMarker();
			if (member) {
				member.SetOnline(false, "");
			}
		}
		// if (GetGame().IsServer()) {
            // OBLLogger.Debug("[OBLParty] Player saiu: " + (GetIdentity() != null ? GetIdentity().GetPlainId() : "sem identidade"));
        // }
	}
	
	void WriteOBLGroupSyncRPC(ScriptRPC rpc) {
		bool present = bxdgroup != null;
		rpc.Write(present);
		if (bxdgroup) {
			bxdgroup.WriteToCtx(rpc);
		}
	}

	void SendGroupInfo() {
		PlayerIdentity ident = GetIdentity();
		if (!ident)
			return;
		string id = ident.GetId();
		int hash = id.Hash();
		identityIdHash = hash;
		SetSynchDirty();
		ScriptRPC rpc = new ScriptRPC();
		WriteOBLGroupSyncRPC(rpc);
		rpc.Send(this, OBLPartyRPCs.GROUP_SYNC, true, ident);
		ScriptRPC rpcMission = new ScriptRPC();
		WriteOBLGroupSyncRPC(rpcMission);
		rpcMission.Send(null, OBLPartyRPCs.GROUP_SYNC, true, ident);
		memberCache = null;
	}
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx) {
		super.OnRPC(sender, rpc_type, ctx);
		if (rpc_type == OBLPartyRPCs.GROUP_RPC) {
			OBLLogger.Debug("Received Group RPC");
			int type = 0;
			if (!ctx.Read(type))
				return;
			OBLParty grp = GetOBLParty();
			OBLLogger.Debug("Received Group RPC Type: " + type + " Group: " + (grp != null));
			if (grp)
				grp.OnRPCServer(sender, type, ctx);
			OBLLogger.Debug("OnRPCServer");
		}
	}

	override void EEKilled(Object killer) {
		super.EEKilled(killer);
		OBLPartyMember member = GetMyGroupMarker();
		if (member)
			member.SetHealth(0.0);
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef) {
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		OBLPartyMember member = GetMyGroupMarker();
		if (member)
			member.SetHealth(GetHealth());
	}
}
