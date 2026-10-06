class OBLPartyPermission {

	int UID, nextGroupUID, previousGroupUID, inheritGroupUID;
	string permName;
	ref array<ref Param2<int, bool>> markerPermissions = new array<ref Param2<int, bool>>();
	bool canUpgrade;
	bool canPromote;
	bool canDemote;
	bool canInvite;
	int promotePower;
	int demotePower;
	int promoteNeedPower;
	int demoteNeedPower;
	bool canDoBasebuilding = false;
	bool canPackPlotpole = false;
	bool tempGroup = false;
	
	[NonSerialized()]
	bool filledInheritedPermissions = false;
	
	
	bool CanPromote(OBLPartyPermission target) {
		if (!target || target.nextGroupUID == -1)
			return false;
		OBLPartyPermission next = target.GetNextGroup();
		if (!next)
			return false;
		return promotePower >= next.promoteNeedPower;
	}
	
	bool CanDemote(OBLPartyPermission target) {
		if (!target || target.previousGroupUID == -1)
			return false;
		return demotePower >= target.demoteNeedPower;
	}
	
	bool CanKick(OBLPartyPermission target) {
		if (!target)
			return false;
		return demotePower >= target.demoteNeedPower;
	}
	
	bool CanSeeMarkerType(OBLMarkerType type) {
		foreach (Param2<int, bool> allowed : markerPermissions) {
			if (allowed.param1 == type)
				return allowed.param2;
		}
		return false;
	}
	
	void FillInherited(OBLPartyPermission inherited) {
		canInvite = canInvite || inherited.canInvite;
		canUpgrade = canUpgrade || inherited.canUpgrade;
		canPromote = canPromote || inherited.canPromote;
		canDoBasebuilding = canDoBasebuilding || inherited.canDoBasebuilding;
		canPackPlotpole = canPackPlotpole || inherited.canPackPlotpole;
		canDemote = canDemote || inherited.canDemote;
		promotePower = Math.Max(promotePower, inherited.promotePower);
		demotePower = Math.Max(demotePower, inherited.demotePower);
		promoteNeedPower = Math.Max(promoteNeedPower, inherited.promoteNeedPower);
		demoteNeedPower = Math.Max(demoteNeedPower, inherited.demoteNeedPower);
		foreach (Param2<int, bool> othermarkerPerms : inherited.markerPermissions) {
			bool found = false;
			foreach (Param2<int, bool> mymarkerPerms : markerPermissions) {
				if (othermarkerPerms.param1 == mymarkerPerms.param1) {
					mymarkerPerms.param2 = mymarkerPerms.param2 || othermarkerPerms.param2;
					found = true;
				}
			}
			if (!found) {
				markerPermissions.Insert(new Param2<int, bool>(othermarkerPerms.param1, othermarkerPerms.param2));
			}
		}
		filledInheritedPermissions = true;
	}
	
	void PrintPermission() {
		if (OBLLogger.IsDebug())
			OBLLogger.Debug(permName + " Temp ? " + tempGroup + " (" + UID + ") < " + previousGroupUID + " > " + nextGroupUID + " : " + inheritGroupUID);
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("canUpgrade: " + canUpgrade + " canPromote: " + canPromote + " canDemote: " + canDemote + " canInvite: " + canInvite + " canPackPlotpole: " + canPackPlotpole + " canDoBasebuilding: " + canDoBasebuilding);
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("promotePower: " + promotePower + " demotePower: " + demotePower + " promoteNeedPower: " + promoteNeedPower + " demoteNeedPower: " + demoteNeedPower);
		foreach (Param2<int, bool> markerPerms : markerPermissions) {
			if (OBLLogger.IsDebug())
				OBLLogger.Debug("Marker Perm: " + markerPerms.param1 + " : " + markerPerms.param2);
		}
	}
	
	void FillInheritedPermissions() {
		if (filledInheritedPermissions)
			return;
		OBLPartyPermission inherited = OBLPartyPermissions.Get().FindPermissionGroupByUID(inheritGroupUID);
		if (inherited) {
			inherited.FillInheritedPermissions();
			FillInherited(inherited);
		}
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(UID);
		ctx.Write(nextGroupUID);
		ctx.Write(previousGroupUID);
		ctx.Write(inheritGroupUID);
		ctx.Write(permName);
		ctx.Write(markerPermissions.Count());
		foreach (Param2<int, bool> perm : markerPermissions) {
			ctx.Write(perm.param1);
			ctx.Write(perm.param2);
		}
		ctx.Write(canUpgrade);
		ctx.Write(canPromote);
		ctx.Write(canDemote);
		ctx.Write(canInvite);
		ctx.Write(canDoBasebuilding);
		ctx.Write(canPackPlotpole);
		ctx.Write(promotePower);
		ctx.Write(demotePower);
		ctx.Write(promoteNeedPower);
		ctx.Write(demoteNeedPower);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(UID))
			return false;
		if (!ctx.Read(nextGroupUID))
			return false;
		if (!ctx.Read(previousGroupUID))
			return false;
		if (!ctx.Read(inheritGroupUID))
			return false;
		if (!ctx.Read(permName))
			return false;
		int count = 0;
		if (!ctx.Read(count))
			return false;
		for (int i = 0; i < count; i++) {
			int type;
			bool canSee = false;
			if (!ctx.Read(type))
				return false;
			if (!ctx.Read(canSee))
				return false;
			markerPermissions.Insert(new Param2<int, bool>(type, canSee));
		}
		if (!ctx.Read(canUpgrade))
			return false;
		if (!ctx.Read(canPromote))
			return false;
		if (!ctx.Read(canDemote))
			return false;
		if (!ctx.Read(canInvite))
			return false;
		if (!ctx.Read(canDoBasebuilding))
			return false;
		if (!ctx.Read(canPackPlotpole))
			return false;
		if (!ctx.Read(promotePower))
			return false;
		if (!ctx.Read(demotePower))
			return false;
		if (!ctx.Read(promoteNeedPower))
			return false;
		if (!ctx.Read(demoteNeedPower))
			return false;
		return true;
	}
	
	OBLPartyPermission GetPreviousGroup() {
		return OBLPartyPermissions.Get().FindPermissionGroupByUID(previousGroupUID);
	}
	
	OBLPartyPermission GetNextGroup() {
		return OBLPartyPermissions.Get().FindPermissionGroupByUID(nextGroupUID);
	}
	
}

class OBLPartyPermissions {

	ref array<ref OBLPartyPermission> allGroups = new array<ref OBLPartyPermission>();
	ref TIntArray tempGroups = new TIntArray;
	
	static ref OBLPartyPermissions g_OBLPartyPermissions;
	
	static void Delete() {
		if (g_OBLPartyPermissions)
			delete g_OBLPartyPermissions;
	}
	
	static OBLPartyPermissions Get() {
		if (!g_OBLPartyPermissions) {
			if (GetGame().IsServer()) {
				g_OBLPartyPermissions = Load();
				g_OBLPartyPermissions.LoadInheritence();
				g_OBLPartyPermissions.LoadTempGroups();
				g_OBLPartyPermissions.PrintAllPermissionGroups();
			} else {
				g_OBLPartyPermissions = new OBLPartyPermissions();
				GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_PERMISSIONS, new Param1<bool>(true), true);
			}
		}
		return g_OBLPartyPermissions;
	}
	
	static OBLPartyPermissions Load() {
		OBLPartyPermissions perms;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_PERMISSIONS)) {
			perms = LoadDefault();
			JsonFileLoader<array<ref OBLPartyPermission>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_PERMISSIONS, perms.allGroups);
			return perms;
		}
		perms = new OBLPartyPermissions();
		OBLLogger.Debug("JsonLoadFile OBLPartyPermissions.json");
		JsonFileLoader<array<ref OBLPartyPermission>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUP_PERMISSIONS, perms.allGroups);
		return perms;
	}
	
	static OBLPartyPermissions LoadDefault() {
		OBLPartyPermissions perms = new OBLPartyPermissions();
		OBLPartyPermission temp = new OBLPartyPermission();
		temp.UID = 1;
		temp.nextGroupUID = 3;
		temp.previousGroupUID = -1;
		temp.inheritGroupUID = -1;
		temp.permName = "Тимчасовий";
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.SERVER_STATIC, true)); // 0
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.SERVER_DYNAMIC, true)); // 1
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.GROUP_PING, true)); // 2
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.GROUP_MARKER, false)); // 3
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.PRIVATE_MARKER, true)); // 4
		temp.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.GROUP_PLAYER_MARKER, true)); // 5
		temp.canPromote = false;
		temp.canDemote = false;
		temp.promotePower = 0;
		temp.demotePower = 0;
		temp.promoteNeedPower = 20;
		temp.demoteNeedPower = 20;
		temp.tempGroup = true;
		temp.canPackPlotpole = false;
		temp.canDoBasebuilding = false;
		perms.allGroups.Insert(temp);
		OBLPartyPermission trial = new OBLPartyPermission();
		trial.UID = 3;
		trial.nextGroupUID = 5;
		trial.previousGroupUID = 1;
		trial.inheritGroupUID = 1;
		trial.permName = "Новачок";
		trial.tempGroup = false;
		perms.allGroups.Insert(trial);
		OBLPartyPermission member = new OBLPartyPermission();
		member.UID = 5;
		member.nextGroupUID = 7;
		member.previousGroupUID = 3;
		member.inheritGroupUID = 3;
		member.permName = "Учасник";
		member.tempGroup = false;
		member.canDoBasebuilding = true;
		member.markerPermissions.Insert(new Param2<int, bool>(OBLMarkerType.GROUP_MARKER, true));
		perms.allGroups.Insert(member);
		OBLPartyPermission admin = new OBLPartyPermission();
		admin.UID = 7;
		admin.nextGroupUID = 9;
		admin.previousGroupUID = 5;
		admin.inheritGroupUID = 5;
		admin.permName = "Адмін";
		admin.tempGroup = false;
		admin.canPromote = true;
		admin.canDemote = true;
		admin.canUpgrade = true;
		admin.canInvite = true;
		admin.canPackPlotpole = true;
		admin.promotePower = 30;
		admin.demotePower = 30;
		admin.promoteNeedPower = 40;
		admin.demoteNeedPower = 40;
		perms.allGroups.Insert(admin);
		OBLPartyPermission leader = new OBLPartyPermission();
		leader.UID = 9;
		leader.nextGroupUID = -1;
		leader.previousGroupUID = 7;
		leader.inheritGroupUID = 7;
		leader.permName = "Лідер";
		leader.tempGroup = false;
		leader.promotePower = 40;
		leader.demotePower = 40;
		leader.promoteNeedPower = 50;
		leader.demoteNeedPower = 50;
		perms.allGroups.Insert(leader);
		return perms;
	}
	
	void PrintAllPermissionGroups() {
		foreach (OBLPartyPermission perm : allGroups) {
			perm.PrintPermission();
		}
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(allGroups.Count());
		foreach (OBLPartyPermission perm : allGroups) {
			perm.WriteToCtx(ctx);
		}
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		int count = 0;
		if (!ctx.Read(count))
			return false;
		allGroups.Clear();
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Reading " + count + " Permission Groups");
		for (int i = 0; i < count; i++) {
			OBLPartyPermission perm = new OBLPartyPermission();
			if (!perm.ReadFromCtx(ctx))
				return false;
			allGroups.Insert(perm);
		}
		return true;
	}
	
	void RPC_OBL(PlayerIdentity sender, ParamsReadContext ctx) {
		if (GetGame().IsServer()) {
			ScriptRPC rpc = new ScriptRPC();
			WriteToCtx(rpc);
			rpc.Send(null, OBLPartyRPCs.CONFIG_SYNC_PERMISSIONS, true, sender);
		} else {
			if (!ReadFromCtx(ctx)) {
				OBLLogger.Warn("Unable to read Permissions Config from Server !");
				OBLLogger.Debug("Groups received: ");
				PrintAllPermissionGroups();
				return;
			}
			if (OBLLogger.IsDebug())
				OBLLogger.Debug("Successfully Received Permissions Config from Server Entries: " + allGroups.Count());
			PrintAllPermissionGroups();
		}
	}
	
	void LoadTempGroups() {
		foreach (OBLPartyPermission perm : allGroups) {
			if (perm.tempGroup)
				tempGroups.Insert(perm.UID);
		}
	}
	
	void LoadInheritence() {
		foreach (OBLPartyPermission perm : allGroups) {
			perm.FillInheritedPermissions();
		}
	}
	
	OBLPartyPermission FindHighestGroup() {
		foreach (OBLPartyPermission perm : allGroups) {
			if (perm.nextGroupUID == -1)
				return perm;
		}
		return null;
	}
	
	OBLPartyPermission FindLowestGroup() {
		foreach (OBLPartyPermission perm : allGroups) {
			if (perm.previousGroupUID == -1)
				return perm;
		}
		return null;
	}
	
	OBLPartyPermission FindPermissionGroupByUID(int uid) {
		if (uid == -1)
			return null;
		foreach (OBLPartyPermission perm : allGroups) {
			if (perm.UID == uid)
				return perm;
		}
		return null;
	}
	
	OBLPartyPermission GetNextPermission(OBLPartyPermission perm) {
		if (!perm)
			return null;
		return FindPermissionGroupByUID(perm.nextGroupUID);
	}
		
	OBLPartyPermission GetNextPermission(int uid) {
		return GetNextPermission(FindPermissionGroupByUID(uid));
	}

	OBLPartyPermission GetPreviousPermission(OBLPartyPermission perm) {
		if (!perm)
			return null;
		return FindPermissionGroupByUID(perm.nextGroupUID);
	}
		
	OBLPartyPermission GetPreviousPermission(int uid) {
		return GetPreviousPermission(FindPermissionGroupByUID(uid));
	}

}
