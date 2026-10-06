modded class OBLPartyMember {
	
	override void SetHealth(float health_) {
		if (GetGame().IsServer()) {
			if (Math.AbsFloat(health_ - this.health) > 1 || (health_ == 0 && this.health != 0) || (health_ == 100 && this.health != 100)) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.HEALTH);
				rpc.Write(health_);
				SendMarkerRPC(rpc);
				super.SetHealth(health_);
			}
		} else {
			super.SetHealth(health_);
		}
	}
	
	override void SetPermission(int perm) {
		if (GetGame().IsServer()) {
			if (perm != permissionGroup) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.PERMISSION);
				rpc.Write(perm);
				SendMarkerRPC(rpc);
				super.SetPermission(perm);
			}
		} else {
			super.SetPermission(perm);
		}
	}
	
	override void SetOnline(bool on, string hashedId_) {
		if (GetGame().IsServer()) {
			if (on != online) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.ONLINE);
				rpc.Write(on);
				rpc.Write(hashedId_);
				SendMarkerRPC(rpc);
			}
		}
		super.SetOnline(on, hashedId_);
	}

}