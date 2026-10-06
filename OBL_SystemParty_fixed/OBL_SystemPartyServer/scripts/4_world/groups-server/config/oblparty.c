modded class OBLParty {

	[NonSerialized()]
	static int currentGroup = 0;
	[NonSerialized()]
	static const int OBL_INVITE_CODE_MIN = 10000;
	[NonSerialized()]
	static const int OBL_INVITE_CODE_MAX = 99999;
	[NonSerialized()]
	static ref map<string, string> bxdInviteCodes = new map<string, string>();
	[NonSerialized()]
	static ref map<string, string> bxdInviteCodeOwners = new map<string, string>();
	[NonSerialized()]
	static ref TStringArray bxdHiddenOnlineSteamids = new TStringArray();
	[NonSerialized()]
	const int GROUP_UPDATE_TIMER = 4000;
	[NonSerialized()]
	ref array<PlayerBase> playerChars = new array<PlayerBase>();
	[NonSerialized()]
	ref array<ref Param2<int, string>> lastInvites = new array<ref Param2<int, string>>();
	// OBL FIX: how long a pending invite stays acceptable (ms). Set to 10000 for 10 seconds.
	[NonSerialized()]
	static const int OBL_INVITE_LIFETIME_MS = 30000;
	// OBL FIX: how long the invite banner is shown on screen (seconds).
	[NonSerialized()]
	static const float OBL_INVITE_BANNER_SECONDS = 8;
	[NonSerialized()]
	static const int OBL_MARKER_NAME_MAX = 64;
	
	override void OnLoadServer() {
		super.OnLoadServer();
		foreach (OBLPartyMember member : members) {
			if (!member)
				continue;
			member.online = false;
			member.currentSubgroup = OBLPartyConstants.SUBGROUP_OFFLINE;
			member.parentGroup = this;
		}
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartUpdateDelayed, (((currentGroup++) * 733) % GROUP_UPDATE_TIMER), false);
		InitNumbers();
		if (GetFreeMemberSlots() < 0) {
			GetGame().AdminLog("Loaded Group has more players, than allowed: " + shortname + " (" + name + ") MemberCount: " + members.Count() + " Max: " + maxPlayers);
		}
	}
	
	void ~OBLParty() {
		if (GetGame()) {
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(StartUpdateDelayed);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateGroup);
			OBLLogger.Debug("Delete Group: " + shortname);
		}
	}
	
	void StartUpdateDelayed() {
		OBLLogger.Debug("Starting Update Delayed " + shortname);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateGroup, GROUP_UPDATE_TIMER, true);
	}
	
	void OnPlayerOnline(PlayerBase player, PlayerIdentity identity) {
		if (!player)
			return;
		lastActivity = JMDate.Now(true).GetTimestamp();
		OBLLogger.Debug("Player Online: " + (identity != null) + " " + (player != null), true);
		if (playerChars.Find(player) == -1)
			playerChars.Insert(player);
		OBLPartyMember member = player.GetMyGroupMarker();
		if (identity && member)
			member.hashedId = identity.GetId();
	}
	
	void OnPlayerOffline(PlayerBase player, string steamid) {
		if (player)
			playerChars.RemoveItem(player);
		lastActivity = JMDate.Now(true).GetTimestamp();
		bool identOk = (player && player.GetIdentity());
		OBLLogger.Debug("Player Offline: " + identOk + " " + (player != null));
		RebuildCache();
	}
	
	void OnPlayerRespawn(PlayerBase player, PlayerIdentity ident) {
		if (player)
			playerChars.RemoveItem(player);
		OBLLogger.Debug("Player Respawn: " + (ident != null) + " " + (player != null));
			
		RebuildCache();
	}
	
	bool CanInvite(string steamid) {
		Param2<int, string> inv = GetInvite(steamid);
		if (!inv)
			return true;
		int lastTime = inv.param1;
		int cooldown = OBLPartyMainConfig.Get().inviteCooldownSeconds * 1000;
		if (cooldown <= 0)
			return true;
		int now = GetGame().GetTime();
		return now - lastTime > cooldown;
	}
	
	bool CanAcceptInvite(string steamid) {
		Param2<int, string> inv = GetInvite(steamid);
		if (!inv)
			return false;
		// OBL FIX: expired invites are no longer acceptable
		return (GetGame().GetTime() - inv.param1) <= OBL_INVITE_LIFETIME_MS;
	}
	
	Param2<int, string> GetInvite(string steamid) {
		OBLLogger.Debug("Searching for invite in " + lastInvites.Count() + " invites");
		foreach (Param2<int, string> invite : lastInvites) {
			if (invite && invite.param2 == steamid) {
				OBLLogger.Debug("Found invite for " + steamid + " : " + invite.param1);
				return invite;
			}
		}
		return null;
	}

	static string GetOrCreateOBLInviteCode(PlayerIdentity identity) {
		EnsureOBLInviteCodeMaps();
		if (!identity)
			return "";
		string steamid = identity.GetPlainId();
		if (steamid == "")
			return "";
		if (bxdInviteCodes.Contains(steamid))
			return bxdInviteCodes.Get(steamid);
		string code = GenerateOBLInviteCode();
		if (code == "")
			return "";
		bxdInviteCodes.Set(steamid, code);
		bxdInviteCodeOwners.Set(code, steamid);
		return code;
	}

	static string ResolveOBLInviteCode(string inviteTarget) {
		EnsureOBLInviteCodeMaps();
		if (bxdInviteCodeOwners.Contains(inviteTarget))
			return bxdInviteCodeOwners.Get(inviteTarget);
		return inviteTarget;
	}

	static string GenerateOBLInviteCode() {
		EnsureOBLInviteCodeMaps();
		string code = "";
		for (int attempts = 0; attempts < 200; attempts++) {
			int randomCode = Math.RandomIntInclusive(OBL_INVITE_CODE_MIN, OBL_INVITE_CODE_MAX);
			code = "" + randomCode;
			if (!bxdInviteCodeOwners.Contains(code))
				return code;
		}
		for (int fallbackCode = OBL_INVITE_CODE_MIN; fallbackCode <= OBL_INVITE_CODE_MAX; fallbackCode++) {
			code = "" + fallbackCode;
			if (!bxdInviteCodeOwners.Contains(code))
				return code;
		}
		return "";
	}

	static void EnsureOBLInviteCodeMaps() {
		if (!bxdInviteCodes)
			bxdInviteCodes = new map<string, string>();
		if (!bxdInviteCodeOwners)
			bxdInviteCodeOwners = new map<string, string>();
	}

	static void SetOBLOnlinePrivacy(string steamid, bool hidden) {
		EnsureOBLOnlinePrivacyList();
		if (steamid == "")
			return;
		int index = bxdHiddenOnlineSteamids.Find(steamid);
		if (hidden) {
			if (index == -1)
				bxdHiddenOnlineSteamids.Insert(steamid);
		} else if (index != -1) {
			bxdHiddenOnlineSteamids.Remove(index);
		}
	}

	static TStringArray GetOBLHiddenOnlineSteamids() {
		EnsureOBLOnlinePrivacyList();
		return bxdHiddenOnlineSteamids;
	}

	static void EnsureOBLOnlinePrivacyList() {
		if (!bxdHiddenOnlineSteamids)
			bxdHiddenOnlineSteamids = new TStringArray();
	}
	
	void AcceptInvite(string steamid, PlayerBase pb) {
		Param2<int, string> inv = GetInvite(steamid);
		if (!inv)
			return;
		lastInvites.RemoveItem(inv);
		AddMember(pb);
	}
	
	void OnRPCServer(PlayerIdentity sender, int type, ParamsReadContext ctx) {
		if (!sender) {
			OBLLogger.Debug("Group RPC without sender ignored. Type: " + type);
			return;
		}
		if (type == OBLPartyRPCs.ADD) {
			OBLMarker marker = new OBLMarker();
			if (!marker.ReadFromCtx(ctx)) {
				OBLLogger.Debug("Failed to read Marker to add !");
				return;
			}
			PlayerBase senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB) {
				OBLLogger.Debug("Failed to get Sender's Player Object !");
				return;
			}
			OBLPartyPermission perms = senderPB.GetPermission();
			if (!perms) {
				OBLLogger.Debug("Failed to get Sender's Permissions !");
				return;
			}
			// OBL FIX: clients may only create group markers / pings (never member markers)
			if (marker.type != OBLMarkerType.GROUP_MARKER && marker.type != OBLMarkerType.GROUP_PING) {
				OBLLogger.Debug("Rejected Marker add with invalid type " + marker.type + " from " + sender.GetPlainId());
				return;
			}
			if (!perms.CanSeeMarkerType(marker.type)) {
				SendErrorNotification(sender, "У вас немає прав для розміщення маркера!");
				return;
			}
			if (marker.name.Length() > OBL_MARKER_NAME_MAX)
				marker.name = marker.name.Substring(0, OBL_MARKER_NAME_MAX);
			// OBL FIX: a client-chosen uid must not collide with an existing marker/member uid
			// (the client's uid is kept otherwise — it uses it to clear its own tactical ping)
			if (marker.uid < 200 || FindAnyMarkerByUID(marker.uid) != null)
				marker.uid = GenerateFreeMarkerUID();
			if (markers.Count() >= markerLimit && marker.type != OBLMarkerType.GROUP_PING) {
				SendErrorNotification(sender, "Досягнуто ліміту маркерів!");
				return;
			}
			AddMarkerServer(marker);
		} else if (type == OBLPartyRPCs.REMOVE) {
			int uid;
			if (!ctx.Read(uid)) {
				OBLLogger.Debug("Failed to receive new Marker ID from Client !");
				return;
			}
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB) {
				OBLLogger.Debug("Failed to get Sender's Player Object !");
				return;
			}
			perms = senderPB.GetPermission();
			if (!perms) {
				OBLLogger.Debug("Failed to get Sender's Permissions !");
				return;
			}
			OBLMarker mark = FindAnyMarkerByUID(uid);
			if (!mark) {
				OBLLogger.Debug("Failed to find Any Marker with uid: " + uid);
				return;
			}
			// OBL FIX: removing a member marker kicked that member (even the leader) without any rank check
			if (mark.type != OBLMarkerType.GROUP_MARKER && mark.type != OBLMarkerType.GROUP_PING) {
				OBLLogger.Debug("Rejected Marker remove of type " + mark.type + " from " + sender.GetPlainId());
				return;
			}
			if (!perms.CanSeeMarkerType(mark.type)) {
				SendErrorNotification(sender, "У вас немає прав для видалення!");
				return;
			}
			RemoveMarkerServer(mark);
		} else if (type == OBLPartyRPCs.INVITE) {
			string invtedSteamid;
			if (!ctx.Read(invtedSteamid)) {
				OBLLogger.Debug("Failed to Receive Invite Steamid !");
				return;
			}
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB) {
				OBLLogger.Debug("Failed to get Sender's Player Object !");
				return;
			}
			perms = senderPB.GetPermission();
			if (!perms) {
				OBLLogger.Debug("Failed to get Sender's Permissions !");
				return;
			}
			invtedSteamid = ResolveOBLInviteCode(invtedSteamid);
			PlayerBase targetPB = GetPlayerBySteamid(invtedSteamid);
			TryInvite(senderPB, targetPB);
		} else if (type == OBLPartyRPCs.JOIN_SUBGROUP) {
			// підгрупа тепер визначається автоматично (онлайн/офлайн), вручну її не змінити
			return;
		} else if (type == OBLPartyRPCs.LEAVE) {
			PlayerBase pb = PlayerBase.GetPlayerByIdentity(sender);
			if (!pb) {
				OBLLogger.Debug("Unable to find Player Object of " + sender.GetPlainId());
				return;
			}
			OBLPartyMember memMarker = pb.GetMyGroupMarker();
			if (!memMarker) {
				OBLLogger.Debug("Unable to find Marker of " + sender.GetPlainId());
				return;
			}
			int permGroup = memMarker.permissionGroup;
			bool wasLastMember = members.Count() <= 1;
			string leftGroupName = name;
			SendInfoNotification(sender, "Ви покинули групу: " + leftGroupName);
			RemoveMarkerServer(memMarker);
			if (!wasLastMember) {
				FindNewLeaderIfNeeded(permGroup);
				OBLPartyManager.Get().SaveGroup(this);
			}
		} else if (type == OBLPartyRPCs.PROMOTE || type == OBLPartyRPCs.DEMOTE || type == OBLPartyRPCs.KICK) {
			string steamid;
			if (!ctx.Read(steamid))
				return;
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB) {
				OBLLogger.Debug("Failed to get Sender's Player Object !");
				return;
			}
			targetPB = GetPlayerBySteamid(steamid);
			perms = senderPB.GetPermission();
			if (!perms) {
				OBLLogger.Debug("Failed to get Sender's Permissions !");
				return;
			}
			OBLPartyMember targetMember = GetMemberBySteamid(steamid);
			if (!targetMember) {
				SendErrorNotification(sender, OBLNotifyTexts.PlayerNotFound());
				return;
			}
			OBLPartyPermission targetPerms = targetMember.GetPermission();
			if (!targetPerms) {
				OBLLogger.Debug("Failed to get Target's Permissions !");
				return;
			}
			if (type == OBLPartyRPCs.PROMOTE) {
				if (!perms.CanPromote(targetPerms)) {
					SendErrorNotification(sender, "У вас немає прав для підвищення!");
					return;
				} else {
					OBLPartyPermission nextPerm = targetPerms.GetNextGroup();
					if (!nextPerm)
						return;
					string newGroupName = nextPerm.permName;
					SendInfoNotification(sender, OBLNotifyTexts.PromotedSender(targetMember.name, newGroupName));
					// OBL FIX: skip the second banner when acting on yourself
					if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, steamid)) {
						SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.PromotedTarget(newGroupName));
					}
					targetMember.SetPermission(nextPerm.UID);
					OBLPartyManager.Get().SaveGroup(this);
					return;
				}
			}
			if (type == OBLPartyRPCs.DEMOTE) {
				if (!perms.CanDemote(targetPerms)) {
					SendErrorNotification(sender, "У вас немає прав для пониження!");
					return;
				} else {
					OBLPartyPermission previousPerm = targetPerms.GetPreviousGroup();
					if (!previousPerm)
						return;
					newGroupName = previousPerm.permName;
					SendInfoNotification(sender, OBLNotifyTexts.DemotedSender(targetMember.name, newGroupName));
					// OBL FIX: skip the second banner when acting on yourself
					if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, steamid)) {
						SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.DemotedTarget(newGroupName));
					}
					targetMember.SetPermission(previousPerm.UID);
					OBLPartyManager.Get().SaveGroup(this);
					return;
				}
			}
			if (type == OBLPartyRPCs.KICK) {
				if (!perms.CanKick(targetPerms)) {
					SendErrorNotification(sender, "У вас немає прав для вигнання!");
					return;
				} else {
					SendInfoNotification(sender, OBLNotifyTexts.KickedSender(targetMember.name));
					// OBL FIX: skip the second banner when acting on yourself
					if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, steamid)) {
						SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.KickedTarget(name));
					}
					bool kickDeletedGroup = members.Count() <= 1;
					int kickedPermGroup = targetMember.permissionGroup;
					RemoveMarkerServer(targetMember);
					if (kickDeletedGroup)
						return;
					// OBL FIX: never leave a group without a leader
					FindNewLeaderIfNeeded(kickedPermGroup);
					OBLPartyManager.Get().SaveGroup(this);
				}
			}
		} else if (type == OBLPartyRPCs.UPGRADE) {
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB) {
				OBLLogger.Debug("Failed to get Sender's Player Object !");
				return;
			}
			perms = senderPB.GetPermission();
			if (!perms) {
				OBLLogger.Debug("Failed to get Sender's Permissions !");
				return;
			}
			if (!perms.canUpgrade) {
				SendErrorNotification(sender, "У вас немає прав для покращення!");
				return;
			}
			pb = PlayerBase.GetPlayerByIdentity(sender);
			if (!pb) {
				OBLLogger.Debug("Unable to find Player Object of " + sender.GetPlainId());
				return;
			}
			// OBL FIX: level used to grow forever (no next-level check)
			if (!OBLPartyLevels.Get().FindLevelByUID(level + 1)) {
				SendErrorNotification(sender, "Досягнуто максимального рівня");
				return;
			}
			GetGame().AdminLog("Player " + sender.GetPlainId() + " (" + sender.GetName() + ") upgraded Group. Name: " + name + " Tag: " + shortname );
			level++;
			OnLevelChanged();
			OBLPartyManager.Get().SaveGroup(this);
			SendInfoNotification(sender, OBLNotifyTexts.GroupUpgraded(level));
		} else if (type == OBLPartyRPCs.CHANGE_TAG_VISIBILITY) {
			bool enabled;
			if (!ctx.Read(enabled)) {
				OBLLogger.Debug("Unable to read enabled From Client !");
				return;
			}
			// OBL FIX: only the leader may change it (same rule the client UI uses)
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB)
				return;
			perms = senderPB.GetPermission();
			if (!perms || perms.nextGroupUID != -1) {
				SendErrorNotification(sender, "Тільки лідер може змінювати показ тегу в чаті!");
				SyncGroupTagVisiblity();
				return;
			}
			showTagInChat = enabled;
			SyncGroupTagVisiblity();
			OBLPartyManager.Get().SaveGroup(this);
		} else if (type > OBLPartyRPCs.START_MARKER_RPC) {
			int uid2;
			if (!ctx.Read(uid2))
				return;
			OBLMarker marker2 = FindAnyMarkerByUID(uid2);
			if (!marker2)
				return;
			// OBL FIX: any member could move any marker, including other members' player markers
			if (marker2.type != OBLMarkerType.GROUP_MARKER && marker2.type != OBLMarkerType.GROUP_PING)
				return;
			senderPB = PlayerBase.GetPlayerByIdentity(sender);
			if (!senderPB)
				return;
			perms = senderPB.GetPermission();
			if (!perms || !perms.CanSeeMarkerType(marker2.type))
				return;
			marker2.OnMarkerRPCServer(type, ctx);
		}
	}
	
	int GenerateFreeMarkerUID() {
		int newUid = Math.RandomInt(200, int.MAX - 1);
		for (int attempt = 0; attempt < 50 && FindAnyMarkerByUID(newUid) != null; attempt++)
			newUid = Math.RandomInt(200, int.MAX - 1);
		return newUid;
	}
	
	override void OnInviteActionServer(PlayerBase inviter, PlayerBase target) {
		TryInvite(inviter, target);
	}
	
	// спільні перевірки для запрошення з меню та через дію біля гравця
	void TryInvite(PlayerBase inviter, PlayerBase targetPB) {
		if (!inviter || !inviter.GetIdentity())
			return;
		PlayerIdentity sender = inviter.GetIdentity();
		if (inviter.GetOBLParty() != this)
			return;
		OBLPartyPermission perms = inviter.GetPermission();
		if (!perms || !perms.canInvite) {
			SendErrorNotification(sender, "У вас немає прав для запрошення!");
			return;
		}
		if (!targetPB || !targetPB.GetIdentity()) {
			SendErrorNotification(sender, OBLNotifyTexts.PlayerNotFound());
			return;
		}
		if (targetPB.GetOBLParty()) {
			SendErrorNotification(sender, OBLNotifyTexts.AlreadyInGroup());
			return;
		}
		if (IsFull()) {
			SendErrorNotification(sender, OBLNotifyTexts.GroupFull());
			return;
		}
		string invitedSteamid = targetPB.GetIdentity().GetPlainId();
		// OBL FIX: enforce the invite cooldown (CanInvite was never called before)
		if (!CanInvite(invitedSteamid)) {
			SendErrorNotification(sender, "Запрошення вже надіслано. Зачекайте, перш ніж надсилати знову.");
			return;
		}
		InvitePlayer(targetPB, sender.GetName());
		SendInfoNotification(sender, "Запрошення надіслано гравцю " + targetPB.GetIdentity().GetName() + ".");
	}
	
	void OnLevelChanged() {
		InitNumbers();
		ScriptRPC upgradeRPC = CreateRPCCall(OBLPartyRPCs.UPGRADE);
		upgradeRPC.Write(level);
		upgradeRPC.Write(maxPlayers);
		upgradeRPC.Write(subGroupCount);
		upgradeRPC.Write(subGroupSize);
		upgradeRPC.Write(markerLimit);
		upgradeRPC.Write(plotpoleLimit);
		SendRPCToGroupMembers(upgradeRPC);
	}
	
	void FindNewLeaderIfNeeded(int leftPermissionGroup) {
		OBLPartyPermission leaderGroup = OBLPartyPermissions.Get().FindHighestGroup();
		if (!leaderGroup)
			return;
		if (leaderGroup.UID != leftPermissionGroup)
			return;
		if (!members || members.Count() == 0)
			return;
		OBLPartyPermission lowerPerm = leaderGroup.GetPreviousGroup();
		bool found = false;
		OBLPartyMember member = null;
		while (lowerPerm) {
			member = GetFirstMembersOfGroup(lowerPerm.UID);
			if (member) {
				found = true;
				break;
			}
			lowerPerm = lowerPerm.GetPreviousGroup();
		}
		if (!found)
			member = members.Get(0);
		if (member)
			member.SetPermission(leaderGroup.UID);
	}
	
	array<ref OBLPartyMember> GetMembersOfGroup(int group) {
		array<ref OBLPartyMember> arr = new array<ref OBLPartyMember>();
		foreach (OBLPartyMember mem : members) {
			if (mem && mem.permissionGroup == group)
				arr.Insert(mem);
		}
		return arr;
	}
	
	OBLPartyMember GetFirstMembersOfGroup(int group) {
		foreach (OBLPartyMember mem : members) {
			if (mem && mem.permissionGroup == group)
				return mem;
		}
		return null;
	}
	
	void InvitePlayer(PlayerBase player, string invitername = "") {
		if (!player)
			return;
		PlayerIdentity ident = player.GetIdentity();
		if (!ident)
			return;
		string inviteSteamid = ident.GetPlainId();
		// OBL FIX: drop any previous pending invite so repeated clicks never stack
		Param2<int, string> pending = GetInvite(inviteSteamid);
		if (pending)
			lastInvites.RemoveItem(pending);
		GetGame().RPCSingleParam(player, OBLPartyRPCs.GROUP_INVITE, new Param1<string>(shortname), true, ident);
		int inviteSeconds = OBL_INVITE_LIFETIME_MS / 1000;
		SendInfoNotification(ident, invitername + " запросив вас у групу " + name + " (" + shortname + ")." + " Натисніть LCtrl + J, щоб прийняти (" + inviteSeconds + " сек).", OBL_INVITE_BANNER_SECONDS);
		lastInvites.Insert(new Param2<int, string>(GetGame().GetTime(), inviteSteamid));
		// OBL FIX: auto-expire the invite so nothing lingers forever
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ExpireInvite, OBL_INVITE_LIFETIME_MS + 250, false, inviteSteamid);
	}
	
	// OBL FIX: removes a stale invite and tells the client to forget it
	void ExpireInvite(string steamid) {
		Param2<int, string> inv = GetInvite(steamid);
		if (!inv)
			return;
		// a newer invite refreshed the timestamp -> its own timer will handle it
		if ((GetGame().GetTime() - inv.param1) < OBL_INVITE_LIFETIME_MS)
			return;
		lastInvites.RemoveItem(inv);
		PlayerBase pb = GetPlayerBySteamid(steamid);
		if (pb && pb.GetIdentity())
			GetGame().RPCSingleParam(pb, OBLPartyRPCs.GROUP_INVITE, new Param1<string>(""), true, pb.GetIdentity());
	}
	
	void InvitePlayer(PlayerIdentity ident, string invitername = "") {
		if (!ident)
			return;
		PlayerBase pb = GetPlayerBySteamid(ident.GetPlainId());
		if (!pb)
			return;
		InvitePlayer(pb, invitername);
	}
	
	void AddMarkerServer(OBLMarker marker) {
		OBLLogger.Debug("Adding Marker Server: " + marker + " Group: " + shortname);
		if (marker) {
			string printname = marker.name + "";
			printname.Replace("%", "");
			OBLLogger.Debug("Marker Info: " + printname + " " + marker.icon + " " + marker.type);
		}
		AddMarkerLocal(marker);
		if (marker.type == OBLMarkerType.GROUP_PING && OBLPartyMainConfig.Get().tacticalPingLifetimeSeconds >= 0)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemovePingMarkerServer, OBLPartyMainConfig.Get().tacticalPingLifetimeSeconds * 1000, false, marker.uid);
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.ADD);
		marker.WriteToCtx(rpc);
		SendRPCToGroupMembers(rpc);
	}
	
	void AddPlayerMarkerServer(OBLMarker marker) {
		OBLLogger.Debug("Adding Player Marker Server: " + marker);
		if (marker) {
			string printname = marker.name + "";
			printname.Replace("%", "");
			OBLLogger.Debug("Marker Info: " + printname + " " + marker.icon + " " + marker.type);
		}
		AddMarkerLocal(marker);
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.ADD_CLIENT);
		marker.WriteToCtx(rpc);
		SendRPCToGroupMembers(rpc);
	}
	
	void RemoveMarkerServer(int uid) {
		OBLMarker marker = FindAnyMarkerByUID(uid);
		if (marker)
			RemoveMarkerServer(marker);
	}
	
	void RemovePingMarkerServer(int uid) {
		OBLMarker marker = FindPingMarkerByUID(uid);
		if (marker)
			RemoveMarkerServer(marker);
	}
	
	void RemoveMarkerServer(OBLMarker marker) {
		if (!marker)
			return;
		OBLLogger.Debug("Removing Marker Server: " + marker);
		if (marker) {
			string printname = marker.name + "";
			printname.Replace("%", "");
			OBLLogger.Debug("Marker Info: " + printname + " " + marker.icon + " " + marker.type);
		}
		int uid = marker.uid;
		bool removedMember = marker.type == OBLMarkerType.GROUP_PLAYER_MARKER;
		RemoveMarkerLocal(marker);
		if (removedMember && (!members || members.Count() == 0)) {
			OBLPartyManager.Get().DeleteGroup(this);
			return;
		}
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.REMOVE);
		rpc.Write(uid);
		SendRPCToGroupMembers(rpc);
		if (removedMember)
			OBLPartyManager.Get().SaveGroup(this);
	}
	
	void RebuildCache() {
	}
	
	void CreateNewGroup(PlayerBase owner, string nam, string tag) {
		OnLoadServer();
		lastActivity = JMDate.Now(true).GetTimestamp();
		creationDate = lastActivity;
		level = 0;
		InitNumbers();
		name = nam;
		shortname = tag;
		OBLPartyPermission leaderPerm = OBLPartyPermissions.Get().FindHighestGroup();
		if (!leaderPerm)
			return;
		int leaderGroup = leaderPerm.UID;
		AddMember(owner, leaderGroup);
	}
	
	void AddMember(PlayerBase pb) {
		OBLPartyPermission lowestPerm = OBLPartyPermissions.Get().FindLowestGroup();
		if (!lowestPerm)
			return;
		int group = lowestPerm.UID;
		AddMember(pb, group);
	}
	
	void AddMember(PlayerBase pb, int permissionGroup) {
		if (!pb)
			return;
		OBLPartyMember member = OBLPartyMember.CreateMember(pb);
		if (!member)
			return;
		member.permissionGroup = permissionGroup;
		member.currentSubgroup = GetNextFreeSubgroup();
		member.online = true;
		if (member.steamid != "")
			OBLPartyManager.Get().cachedGroups.Set(member.steamid, this);
		AddPlayerMarkerServer(member);
		pb.InitGroupServer(pb.GetIdentity(), this);
	}
	
	override void RemoveMember(OBLPartyMember member) {
		if (!member)
			return;
		string steamid = member.steamid;
		if (steamid != "")
			OBLPartyManager.Get().cachedGroups.Set(steamid, null);
		//RemoveMarkerServer(member);
		members.RemoveItem(member);
		delete member;
		PlayerBase pb = GetPlayerBySteamid(steamid);
		if (pb) {
			OnPlayerOffline(pb, steamid);
			pb.ClearGroupServer(this);
		}
	}
	
	static PlayerBase GetPlayerBySteamid(string steamid) {
		ref array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		foreach (Man player : players) {
			PlayerBase pb = PlayerBase.Cast(player);
			if (!pb || !pb.GetIdentity())
				continue;
			if (pb.GetIdentity().GetPlainId() == steamid)
				return pb;
		}
		return null;
	}
	
	int GetNextFreeSubgroup() {
		// новий учасник щойно в грі — він онлайн
		return OBLPartyConstants.SUBGROUP_ONLINE;
	}
	
	// ліміт гравців: 6 за замовчуванням або значення адміна (до 15)
	int GetMaxPlayersLimit() {
		if (maxPlayersOverride > 0)
			return Math.Min(maxPlayersOverride, OBLPartyConstants.MAX_PLAYERS_LIMIT);
		return OBLPartyConstants.DEFAULT_MAX_PLAYERS;
	}
	
	override void InitNumbers() {
		super.InitNumbers();
		OBLPartyLevel lvl = OBLPartyLevels.Get().FindLevelByUID(level);
		if (!lvl)
			lvl = OBLPartyLevels.Get().GetHighestLevel();
		maxPlayers = GetMaxPlayersLimit();
		subGroupCount = OBLPartyConstants.SUBGROUP_COUNT;
		subGroupSize = maxPlayers;
		markerLimit = OBLPartyMainConfig.Get().groupMarkerLimit;
		plotpoleLimit = 0;
		if (lvl) {
			markerLimit += lvl.groupMarkerLimitAdded;
			plotpoleLimit = lvl.groupPlotpolesLimitAdded;
		}
	}
	
	int GetFreeMemberSlots() {
		return maxPlayers - members.Count();
	}
	
	bool IsFull() {
		return GetFreeMemberSlots() <= 0;
	}
	
	void UpdatePlayerList() {
		for (int i = 0; i < playerChars.Count(); i++) {
			if (!playerChars.Get(i) || !playerChars.Get(i).IsAlive() || !playerChars.Get(i).GetIdentity())
				playerChars.Remove(i--);
		}
	}
	
	void UpdateGroup() {
		if (playerChars && playerChars.Count() > 0) { // Reduce Log Spam. There is nothing to update if no player is online
			OBLLogger.Debug("Update Group: " + shortname);
			UpdatePlayerList();
			UpdatePlayerPositionsAndHealth();
			OBLLogger.Debug("Finished Update Group: " + shortname);
		}
	}
	
	void UpdatePlayerPositionsAndHealth() {
		foreach (PlayerBase player : playerChars) {
			if (!player || !player.GetIdentity())
				continue;
			OBLPartyMember member = GetMemberBySteamid(player.GetIdentity().GetPlainId());
			if (member) {
				member.SetPosition(player.GetPosition());
				member.SetHealth(player.GetHealth());
			}
		}
	}
	
	void SendErrorNotification(PlayerIdentity player, string message, float show_time = 4) {
		if (!player)
			return;
		string printmessage = message + "";
		printmessage.Replace("%", "");
		// OBL FIX: '%' breaks widget text formatting; clamp runaway show_time
		if (show_time <= 0 || show_time > 15)
			show_time = 4;
		OBLLogger.Debug("Sending Error Notification to " + player.GetPlainId() + ": " + printmessage);
		NotificationSystem.SendNotificationToPlayerIdentityExtended(player, show_time, "Система груп", printmessage, "set:ccgui_enforce image:MapDestroyed");
	}
	
	void SendInfoNotification(PlayerIdentity player, string message, float show_time = 4) {
		if (!player)
			return;
		string printmessage = message + "";
		printmessage.Replace("%", "");
		// OBL FIX: '%' breaks widget text formatting; clamp runaway show_time
		if (show_time <= 0 || show_time > 15)
			show_time = 4;
		OBLLogger.Debug("Sending Info Notification to " + player.GetPlainId() + ": " + printmessage);
		NotificationSystem.SendNotificationToPlayerIdentityExtended(player, show_time, "Система груп", printmessage, "set:ccgui_enforce image:HudUserMarker");
	}
	
	void SyncGroupTagVisiblity() {
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.CHANGE_TAG_VISIBILITY);
		rpc.Write(showTagInChat);
		SendRPCToGroupMembers(rpc);
	}
	
	override void SendRPCToGroupMembers(ScriptRPC rpc, int otherRPCType = -1) {
		if (!playerChars)
			playerChars = new array<PlayerBase>();
		OBLLogger.Debug("Sending RPC To Group Members: " + playerChars.Count());
		int type = otherRPCType;
		if (type == -1)
			type = OBLPartyRPCs.GROUP_RPC;
		int count = 0;
		foreach (PlayerBase pb : playerChars) {
			if (!pb)
				continue;
			PlayerIdentity ident = pb.GetIdentity();
			if (ident) {
				rpc.Send(null, type, true, ident);
				count++;
				string printname = ident.GetName() + "";
				printname.Replace("%", "");
				OBLLogger.Debug("RPC Tpye: " + type + " Sent to " + printname + " " + ident.GetPlainId());
			}
		}
		OBLLogger.Debug("RPC was sent to " + count + " Players");
	}
	
	void ResendGroupInfo() {
		foreach (PlayerBase player : playerChars) {
			if (!player)
				continue;
			player.SendGroupInfo();
		}
	}
}
