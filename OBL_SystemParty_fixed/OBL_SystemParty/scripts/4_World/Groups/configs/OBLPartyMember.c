class OBLPartyMember : OBLMarker {

	string steamid;
	int permissionGroup;
	bool online = false;
	float health;
	[NonSerialized()]
	int lastPBCheck = 0;
	[NonSerialized()]
	PlayerBase clientPBFound;
	[NonSerialized()]
	string hashedId = "";
	
	void FindPlayerBase() {
		//ref array<Man> players = ClientData.m_PlayerBaseList;
		ref array<PlayerBase> players = PlayerBase.bxd_player_list;
		OBLLogger.Debug("Check PlayerBase: " + name + ". List Size: " + PlayerBase.bxd_player_list.Count());
		int hashidhash = hashedId.Hash(); // Second Identifier as a backup plan if the Identity gets deleted from Helicopter Mod
		foreach (PlayerBase player : players) {
			if (player && player.IsAlive() && player.GetIdentity()) {
				string hashid = player.GetIdentity().GetId();
				if (hashedId.Length() > 0 && (hashid == hashedId || hashidhash == player.identityIdHash)) {
					clientPBFound = player;
					return;
				}
			}
		}
	}
	
	OBLPartyPermission GetPermission() {
		return OBLPartyPermissions.Get().FindPermissionGroupByUID(permissionGroup);
	}
	
	override void SetPosition(vector pos) {
		if (!IsValidPlayerBase())
			super.SetPosition(pos);
	}
	
	vector GetHeadPos() {
		vector vec = "0 0 0";
		MiscGameplayFunctions.GetHeadBonePos(clientPBFound, vec);
		vec = vec + "0 0.2 0";
		return vec;
	}
	
	override bool ShouldCenterWidget() {
		return true;
	}
	
	override bool ShowMarker() {
		if (!mainWidget || !GetGame() || !GetGame().GetPlayer() || !GetGame().GetPlayer().IsAlive() || GetGame().GetPlayer().IsUnconscious())
			return false;
		//OBLLogger.Debug("Marker Show was ok. My Steamid: " + MissionBaseWorld.mySteamid + " This Steamid: " + steamid + " Name: " + name + " Online: " + online);
		if (MissionBaseWorld.mySteamid.Length() == 17 && MissionBaseWorld.mySteamid == steamid)
			return false;
		if (!online)
			return false;

		vector camPos = GetGame().GetCurrentCameraPosition();
		dist = vector.Distance(position, camPos);

		int clientDistance = OBLMarkerVisibilityManager.Get().GetTeammate3DMarkerDistance();
		if (clientDistance == 0)
			return false;
		if (clientDistance > 0 && dist > clientDistance)
			return false;

		return super.ShowMarker();
	}
	
	override bool UpdateMarkerClient() {
		if (!super.UpdateMarkerClient())
			return false;
		int now = GetGame().GetTime();
		if (now - lastPBCheck > 1000) { // Always look for new Playerbase to see if that fixes invisible Tags related to Cars and Helicopters
			FindPlayerBase();
			lastPBCheck = now;
		}
		if (IsValidPlayerBase()) {
			position = GetHeadPos();
		}
		return true;
	}
	
	override void OnMarkerRPCClient(int type_, ParamsReadContext ctx) {
		super.OnMarkerRPCClient(type_, ctx);
		if (type_ == OBLPartyRPCs.HEALTH) {
			float health_ = 0;
			if (!ctx.Read(health_))
				return;
			SetHealth(health_);
		} else if (type_ == OBLPartyRPCs.PERMISSION) {
			int perm = 0;
			if (!ctx.Read(perm))
				return;
			SetPermission(perm);
		} else if (type_ == OBLPartyRPCs.ONLINE) {
			bool online_ = 0;
			string hashedId_;
			if (!ctx.Read(online_))
				return;
			if (!ctx.Read(hashedId_))
				return;
			SetOnline(online_, hashedId_);
		}
	}
	
	void SetPermission(int perm) {
		permissionGroup = perm;
	}
	
	void SetOnline(bool on, string hashedId_) {
		online = on;
		this.hashedId = hashedId_;
		if (GetGame().IsClient())
			SetColor();
	}
	
	void SetHealth(float health_) {
		this.health = health_;
	}
	
	bool IsValidPlayerBase() {
		if (!clientPBFound || !clientPBFound.IsAlive())
			return false;
		return true;
	}
	
	static OBLPartyMember CreateMember(PlayerBase pb) {
		if (!pb)
			return null;
		PlayerIdentity ident = pb.GetIdentity();
		if (!ident)
			return null;
		OBLPartyMember member = new OBLPartyMember();
		member.steamid = ident.GetPlainId();
		member.hashedId = ident.GetId();
		member.SetupMarker(OBLMarkerType.GROUP_PLAYER_MARKER, ident.GetName(), "", pb.GetPosition());
		return member;
	}
	
	override int GetColorARGB() {
		if (online) {
			return OBLColorManager.Get().GetColor("Player Online");
		} else {
			return OBLColorManager.Get().GetColor("Player Offline");
		}
	}
	
	override int Get3DColorARGB() {
		if (online) {
			return OBLColorManager.Get().GetColor("Player 3D Marker");
		} else {
			return super.Get3DColorARGB();
		}
	}
	
	override bool ReadFromCtx(ParamsReadContext ctx) {
		if (!super.ReadFromCtx(ctx))
			return false;
		if (!ctx.Read(steamid))
			return false;
		if (!ctx.Read(hashedId))
			return false;
		if (!ctx.Read(permissionGroup))
			return false;
		if (!ctx.Read(currentSubgroup))
			return false;
		if (!ctx.Read(online))
			return false;
		if (!ctx.Read(health))
			return false;
		icon = OBLPartyMainConfig.Get().otherPlayerIconsPath;
		return true;
	}
	
	override void WriteToCtx(ParamsWriteContext ctx) {
		super.WriteToCtx(ctx);
		ctx.Write(steamid);
		ctx.Write(hashedId);
		ctx.Write(permissionGroup);
		ctx.Write(currentSubgroup);
		ctx.Write(online);
		ctx.Write(health);
	}

}