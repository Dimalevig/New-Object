modded class OBLMarker {

	override void SetPosition(vector pos) {
		if (GetGame().IsServer()) {
			if (vector.Distance(pos, position) > 1) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.POSITION);
				rpc.Write(pos[0]);
				rpc.Write(pos[1]);
				rpc.Write(pos[2]);
				SendMarkerRPC(rpc);
				super.SetPosition(pos);
			}
		} else {
			super.SetPosition(pos);
		}
	}
	override void SetName(string name_) {
		if (GetGame().IsServer()) {
			if (this.name != name_) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.NAME);
				rpc.Write(name_);
				SendMarkerRPC(rpc);
				super.SetName(name_);
			}
		} else {
			super.SetName(name_);
		}
	}
	
	override void SetSubGroup(int grp) {
		if (GetGame().IsServer()) {
			if (grp != currentSubgroup) {
				ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.SUBGRUOP);
				rpc.Write(grp);
				SendMarkerRPC(rpc);
				super.SetSubGroup(grp);
			}
		} else {
			super.SetSubGroup(grp);
		}
	}
	
	void OnMarkerRPCServer(int type_, ParamsReadContext ctx) {
		if (type_ == OBLPartyRPCs.POSITION) {
			float x,y,z;
			if (!ctx.Read(x) || !ctx.Read(y) || !ctx.Read(z))
				return;
			vector vec = Vector(x,y,z);
			SetPosition(vec);
			if (parentGroup && type == OBLMarkerType.GROUP_MARKER)
				parentGroup.saveDirty = true;
		}
	}
}