modded class OBLPartyManager {

	const int SAVE_ITERATION_COUNT = 10;
	const int SAVE_GROUP_INTERVAL = 30 * 1000;
	const int MIN_LENGTH_NAME = 5;
	const int MIN_LENGTH_TAG = 2;
	const int MAX_LENGTH_NAME = 20;
	const int MAX_LENGTH_TAG = 5;
	const string ALLOWED_CHARACTERS_TAG = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const string ALLOWED_CHARACTERS_NAME = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _";
	
	ref array<ref OBLParty> allGroups = new array<ref OBLParty>();
	ref map<string, ref OBLParty> cachedGroups = new map<string, ref OBLParty>();
	int currentSaveIndex = -1;
	
	
	void OBLPartyManager() {
		GetDayZGame().Event_OnRPC.Insert(OnRPC_GroupMgr);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SaveNextGroups, SAVE_GROUP_INTERVAL, true);
	}
	
	void ~OBLPartyManager() {
		GetDayZGame().Event_OnRPC.Remove(OnRPC_GroupMgr);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(SaveNextGroups);
		SaveAllGroups();
	}
	
	void SaveAllGroups() {
		OBLLogger.Debug("Saving all " + allGroups.Count() + " Groups");
		for (int i = allGroups.Count() - 1; i >= 0; i--) {
			OBLParty grp = allGroups.Get(i);
			SaveGroup(grp);
		}
		OBLLogger.Debug("Saved all " + allGroups.Count() + " Groups");
	}
	
	void SaveNextGroups() {
		if (allGroups.Count() < SAVE_ITERATION_COUNT) {
			SaveAllGroups();
			return;
		}
		OBLLogger.Debug("Saving Next " + SAVE_ITERATION_COUNT + " Groups. Start: " + currentSaveIndex);
		for (int i = 0; i < SAVE_ITERATION_COUNT; i++) {
			if (allGroups.Count() == 0) {
				currentSaveIndex = -1;
				return;
			}
			currentSaveIndex = (currentSaveIndex + 1) % allGroups.Count();
			OBLParty grp = allGroups.Get(currentSaveIndex);
			SaveGroup(grp);
		}
		OBLLogger.Debug("Saved " + SAVE_ITERATION_COUNT + " Groups. Next Index: " + currentSaveIndex);
	}
	
	void SerializeAllGroups(ParamsWriteContext ctx) {
		ctx.Write(allGroups.Count());
		foreach (OBLParty grp : allGroups) {
			bool notNull = grp != NULL;
			ctx.Write(notNull);
			if (notNull) {
				grp.WriteToCtx(ctx);
			}
		}
	}
	
	OBLParty GetGroupByShortName(string shortname) {
		foreach (OBLParty grp : allGroups) {
			if (grp && grp.shortname == shortname)
				return grp;
		}
		return null;
	}
	
	OBLParty GetPlayersGroup(string steamid) {
		if (steamid == "")
			return null;
		if (cachedGroups.Contains(steamid)) {
			OBLParty grp = cachedGroups.Get(steamid);
			if (grp && allGroups.Find(grp) != -1 && grp.IsMember(steamid)) 
				return grp;
		}
		grp = FindPlayerGroup(steamid);
		cachedGroups.Set(steamid, grp);
		return grp;
	}
	
	OBLParty FindPlayerGroup(string steamid) {
		foreach (OBLParty grp : allGroups) {
			if (!grp)
				continue;
			if (grp.IsMember(steamid))
				return grp;
		}
		return null;
	}
	
	void OnRPC_GroupMgr(PlayerIdentity sender, Object object, int rpc_type, ParamsReadContext ctx) {
		if (!sender)
			return;
		OBLParty grp = null;
		if (rpc_type == OBLPartyRPCs.GROUP_CREATE) {
			OBLLogger.Debug("Group Create RPC received. " + rpc_type);
			Param2<string, string> createParam;
			PlayerBase pb = PlayerBase.GetPlayerByIdentity(sender);
			if (!pb) {
				SendErrorNotification(sender, "Внутрішня помилка 001 !");
				return;
			}
			if (pb.GetOBLParty()) {
				SendErrorNotification(sender, "Ви вже перебуваєте у групі!");
				return;
			}
			if (!ctx.Read(createParam)) {
				OBLLogger.Debug("Failed to receive Group Create Param");
				return;
			}
			string name = createParam.param1;
			string tag = createParam.param2;
			if (name.Length() < MIN_LENGTH_NAME || name.Length() > MAX_LENGTH_NAME) {
				SendErrorNotification(sender, "Назва групи має бути від " + MIN_LENGTH_NAME + " до " + MAX_LENGTH_NAME + " символів!");
				return;
			}
			if (tag.Length() < MIN_LENGTH_TAG || tag.Length() > MAX_LENGTH_TAG) {
				SendErrorNotification(sender, "Тег групи має бути від " + MIN_LENGTH_TAG + " до " + MAX_LENGTH_TAG + " символів!");
				return;
			}
			if (ConainsSpecialChars(name, ALLOWED_CHARACTERS_NAME) || ConainsSpecialChars(tag, ALLOWED_CHARACTERS_TAG)) {
				SendErrorNotification(sender, "Використано недопустимі символи!");
				return;
			}
			if (GroupNameTaken(name)) {
				SendErrorNotification(sender, "Ця назва вже зайнята!");
				return;
			}
			if (GroupTagTaken(tag)) {
				SendErrorNotification(sender, "Цей тег вже зайнятий!");
				return;
			}
			grp = new OBLParty();
			allGroups.Insert(grp);
			grp.CreateNewGroup(pb, name, tag);
			pb.SendGroupInfo();
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(pb.SendGroupInfo, 750, false);
			GetGame().AdminLog("Player " + sender.GetPlainId() + " (" + sender.GetName() + ") created a new Group. Name: " + name + " Tag: " + tag);
			SaveGroup(grp);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ACCEPT_INVITE) {
			Param1<string> groupNameParam;
			if (!ctx.Read(groupNameParam)) {
				OBLLogger.Debug("Unable to read Groupname from Invite Accept");
				return;
			}
			OBLParty targetGroup = GetGroupByShortName(groupNameParam.param1);
			if (!targetGroup) {
				OBLLogger.Debug("Player Accepted Invite to an unknown Group: " + groupNameParam.param1 + " " + sender.GetPlainId());
				SendErrorNotification(sender, "Групу не знайдено!");
				return;
			}
			if (!targetGroup.CanAcceptInvite(sender.GetPlainId())) {
				OBLLogger.Debug("Player Accepted Invite which was not valid: " + groupNameParam.param1 + " " + sender.GetPlainId());
				SendErrorNotification(sender, "Запрошення не знайдено або воно недійсне!");
				return;
			}
			if (targetGroup.IsFull()) {
				OBLLogger.Debug("Player tried to join full group: " + groupNameParam.param1 + " " + sender.GetPlainId());
				SendErrorNotification(sender, OBLNotifyTexts.GroupFull());
				return;
			}
			
			pb = PlayerBase.GetPlayerByIdentity(sender);
			if (!pb) {
				OBLLogger.Debug("Player Not found ! Error 002");
				SendErrorNotification(sender, "Внутрішня помилка 002 !");
				return;
			}
			targetGroup.AcceptInvite(sender.GetPlainId(), pb);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_LIST) {
			if (!sender)
				return;
			if (!OBLPartyMainConfig.Get().IsAdmin(sender)) {
				GetGame().AdminLog("Player Requested Group Admin List, but is not admin ! " + sender.GetPlainId());
				return;
			}
			OBLLogger.Debug("All Groups Requested by " + sender.GetPlainId());
			SendAllGroupsAdmin(sender);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_DELETE) {
			string grpShortname = "";
			if (!ReadGroupFromCtxAdminCheck(sender, ctx, grp, grpShortname))
				return;
			GetGame().AdminLog("Admin Group Delete Request ! " + sender.GetPlainId() + " for " + grpShortname);
			SendInfoNotification(sender, "Групу " + grpShortname + " видалено");
			DeleteGroup(grp);
			GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_ADMIN_DELETE, new Param1<string>(grpShortname), true, sender);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_NAMES) {
			string grpShortname2 = "";
			if (!ReadGroupFromCtxAdminCheck(sender, ctx, grp, grpShortname2))
				return;
			string newshortname, newdisplayname;
			if (!ctx.Read(newshortname) || !ctx.Read(newdisplayname))
				return;
			if (newdisplayname.Length() < MIN_LENGTH_NAME || newdisplayname.Length() > MAX_LENGTH_NAME) {
				SendErrorNotification(sender, "Назва групи має бути від " + MIN_LENGTH_NAME + " до " + MAX_LENGTH_NAME + " символів!");
				return;
			}
			if (newshortname.Length() < MIN_LENGTH_TAG || newshortname.Length() > MAX_LENGTH_TAG) {
				SendErrorNotification(sender, "Тег групи має бути від " + MIN_LENGTH_TAG + " до " + MAX_LENGTH_TAG + " символів!");
				return;
			}
			if (ConainsSpecialChars(newdisplayname, ALLOWED_CHARACTERS_NAME) || ConainsSpecialChars(newshortname, ALLOWED_CHARACTERS_TAG)) {
				SendErrorNotification(sender, "Використано недопустимі символи!");
				return;
			}
			if (GroupNameTaken(newdisplayname, grp)) {
				SendErrorNotification(sender, "Ця назва вже зайнята!");
				return;
			}
			if (GroupTagTaken(newshortname, grp)) {
				SendErrorNotification(sender, "Цей тег вже зайнятий!");
				return;
			}
			string oldshortname = grp.shortname;
			DeleteGroup(grp.shortname);
			grp.shortname = newshortname;
			grp.name = newdisplayname;
			SaveGroup(grp);
			grp.ResendGroupInfo();
			GetGame().AdminLog("Admin Group Name Change Request ! " + sender.GetPlainId() + " for " + grpShortname2 + " Newshortname: " + newshortname + " NewDisplayname: " + newdisplayname);
			SendInfoNotification(sender, "Назву групи змінено");
			RefreshGroupAdmin(sender, grp, oldshortname);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_LEVEL) {
			string grpShortname3 = "";
			if (!ReadGroupFromCtxAdminCheck(sender, ctx, grp, grpShortname3))
				return;
			int targetLevel = 0;
			if (!ctx.Read(targetLevel))
				return;
			OBLPartyLevel lvl = OBLPartyLevels.Get().FindLevelByUID(targetLevel);
			if (targetLevel < 0 || (!lvl && targetLevel > 0)) {
				SendErrorNotification(sender, "Досягнуто максимального рівня");
				return;
			}
			grp.level = targetLevel;
			grp.OnLevelChanged();
			SaveGroup(grp);
			SendInfoNotification(sender, OBLNotifyTexts.GroupUpgraded(targetLevel));
			RefreshGroupAdmin(sender, grp);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_JOIN) {
			string grpShortname4 = "";
			if (!ReadGroupFromCtxAdminCheck(sender, ctx, grp, grpShortname4))
				return;
			PlayerBase targetPB = OBLParty.GetPlayerBySteamid(sender.GetPlainId());
			if (!targetPB)
				return;
			if (targetPB.GetOBLParty()) {
				SendErrorNotification(sender, OBLNotifyTexts.AlreadyInGroup());
				return;
			}
			grp.AddMember(targetPB);
			RefreshGroupAdmin(sender, grp);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_KICK || rpc_type == OBLPartyRPCs.GROUP_ADMIN_DEMOTE || rpc_type == OBLPartyRPCs.GROUP_ADMIN_PROMOTE || rpc_type == OBLPartyRPCs.GROUP_ADMIN_TOLEADER) {
			string grpShortname5 = "";
			if (!ReadGroupFromCtxAdminCheck(sender, ctx, grp, grpShortname5))
				return;
			string targetSteamid = "";
			if (!ctx.Read(targetSteamid))
				return;
			OBLPartyMember member = grp.GetMemberBySteamid(targetSteamid);
			if (!member) {
				SendErrorNotification(sender, OBLNotifyTexts.PlayerNotFound());
				return;
			}
			OBLPartyPermission targetPerms = member.GetPermission();
			if (!targetPerms) {
				OBLLogger.Debug("Failed to get Target's Permissions !");
				return;
			}
			targetPB = OBLParty.GetPlayerBySteamid(targetSteamid);
			if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_KICK) {
				SendInfoNotification(sender, OBLNotifyTexts.KickedSender(member.name));
				// OBL FIX: skip the second banner when acting on yourself
				if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, targetSteamid)) {
					SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.KickedTarget(grp.name));
				}
				oldshortname = grp.shortname;
				bool deletedByKick = grp.members.Count() <= 1;
				int kickedPermGroup = member.permissionGroup;
				grp.RemoveMarkerServer(member);
				if (deletedByKick) {
					GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_ADMIN_DELETE, new Param1<string>(oldshortname), true, sender);
					return;
				}
				// OBL FIX: never leave a group without a leader
				grp.FindNewLeaderIfNeeded(kickedPermGroup);
			} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_DEMOTE) {
				OBLPartyPermission newPerms = targetPerms.GetPreviousGroup();
				if (!newPerms) {
					SendErrorNotification(sender, OBLNotifyTexts.RankLowerNotFound());
					return;
				}
				SendInfoNotification(sender, OBLNotifyTexts.DemotedSender(member.name, newPerms.permName));
				// OBL FIX: skip the second banner when acting on yourself
				if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, targetSteamid)) {
					SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.DemotedTarget(newPerms.permName));
				}
				member.SetPermission(newPerms.UID);
			} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_PROMOTE) {
				newPerms = targetPerms.GetNextGroup();
				if (!newPerms) {
					SendErrorNotification(sender, OBLNotifyTexts.RankHigherNotFound());
					return;
				}
				// OBL FIX: skip the second banner when acting on yourself
				if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, targetSteamid)) {
					SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.PromotedTarget(newPerms.permName));
				}
				SendInfoNotification(sender, OBLNotifyTexts.PromotedSender(member.name, newPerms.permName));
				member.SetPermission(newPerms.UID);
			} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_TOLEADER) {
				newPerms = OBLPartyPermissions.Get().FindHighestGroup();
				if (!newPerms) {
					SendErrorNotification(sender, OBLNotifyTexts.RankLeaderNotFound());
					return;
				}
				// OBL FIX: skip the second banner when acting on yourself
				if (targetPB && targetPB.GetIdentity() && !OBLNotifyTexts.IsSelfAction(sender, targetSteamid)) {
					SendInfoNotification(targetPB.GetIdentity(), OBLNotifyTexts.PromotedTarget(newPerms.permName));
				}
				SendInfoNotification(sender, OBLNotifyTexts.PromotedSender(member.name, newPerms.permName));
				member.SetPermission(newPerms.UID);
			}
			SaveGroup(grp);
			RefreshGroupAdmin(sender, grp);
		}
	}
	
	void RefreshGroupAdmin(PlayerIdentity sender, OBLParty grp, string shortname = "") {
		if (!grp || !sender)
			return;
		if (shortname == "")
			shortname = grp.shortname;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(shortname);
		grp.WriteToCtx(rpc);
		rpc.Send(NULL, OBLPartyRPCs.GROUP_ADMIN_LIST_SINGLE, true, sender);
	}
	
	void SendAllGroupsAdmin(PlayerIdentity sender) {
		ScriptRPC rpc = new ScriptRPC();
		SerializeAllGroups(rpc);
		rpc.Send(NULL, OBLPartyRPCs.GROUP_ADMIN_LIST, true, sender);
	}
	
	bool ReadGroupFromCtxAdminCheck(PlayerIdentity sender, ParamsReadContext ctx, out OBLParty grp, out string grpShortname2) {
		if (!sender)
			return false;
		if (!OBLPartyMainConfig.Get().IsAdmin(sender)) {
			GetGame().AdminLog("Player Requested Group Admin change, but is not admin ! " + sender.GetPlainId());
			return false;
		}
		if (!ctx.Read(grpShortname2))
			return false;
		grp = GetGroupByShortName(grpShortname2);
		if (!grp)
			return false;
		return true;
	}
	
	bool GroupNameTaken(string name, OBLParty exception = null) {
		foreach (OBLParty grp : allGroups) {
			if (!grp)
				continue;
			if (grp == exception)
				continue;
			if (grp.name == name) {
				return true;
			}
		}
		return false;
	}
	
	override bool GroupTagTaken(string tag, OBLParty exception = null) {
		if (super.GroupTagTaken(tag, exception))
			return true;
		string lower2 = tag + "";
		lower2.ToLower();
		foreach (OBLParty grp : allGroups) {
			if (!grp)
				continue;
			if (grp == exception)
				continue;
			string lower1 = grp.shortname + "";
			lower1.ToLower();
			if (lower1 == lower2) {
				return true;
			}
		}
		string tagUpper = tag + "";
		tagUpper.ToUpper();
		int indexFilename = illegalFilenames.Find(tagUpper);
		if (indexFilename != -1) {
			string illegalFN2 = illegalFilenames.Get(indexFilename);
			return true;
		}
		if (tagUpper.Length() == 4) {
			if (!ConainsSpecialChars(tagUpper.Substring(3, 1), "0123456789")) {
				foreach (string illegalFN : illegalFilenamesNum) {
					if (tagUpper.IndexOf(illegalFN) == 0) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	bool ConainsSpecialChars(string str, string allowed) {
		for (int i = 0; i < str.Length(); i++) {
			if (allowed.IndexOf(str[i]) == -1)
				return true;
		}
		return false;
	}
	
	void SendErrorNotification(PlayerIdentity player, string message, int show_time = 4) {
		if (!player)
			return;
		// OBL FIX: '%' breaks widget text formatting; clamp runaway show_time
		string safeError = message + "";
		safeError.Replace("%", "");
		if (show_time <= 0 || show_time > 15)
			show_time = 4;
		NotificationSystem.SendNotificationToPlayerIdentityExtended(player, show_time, "Система груп", safeError, "set:ccgui_enforce image:MapDestroyed");
	}
	void SendInfoNotification(PlayerIdentity player, string message, int show_time = 4) {
		if (!player)
			return;
		// OBL FIX: '%' breaks widget text formatting; clamp runaway show_time
		string safeInfo = message + "";
		safeInfo.Replace("%", "");
		if (show_time <= 0 || show_time > 15)
			show_time = 4;
		NotificationSystem.SendNotificationToPlayerIdentityExtended(player, show_time, "Система груп", safeInfo, "set:ccgui_enforce image:HudUserMarker");
	}
	
	override void SaveGroup(OBLParty grp) {
		if (!grp)
			return;
		if (!grp.members)
			grp.members = new array<ref OBLPartyMember>();
		if (!grp.members || grp.members.Count() == 0) {
			DeleteGroup(grp);
			return;
		}
		for (int i = 0; i < grp.members.Count(); i++) {
			if (!grp.members.Get(i)) {
				grp.members.Remove(i);
				i--;
			}
		}
		if (grp.members.Count() == 0) {
			DeleteGroup(grp);
			return;
		}
		string path = OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER + grp.shortname + ".json";
		JsonFileLoader<OBLParty>.JsonSaveFile(path, grp);
	}
	
	override void LoadAllGroups() {
		super.LoadAllGroups();
		string filename;
		FileAttr attr;
		FindFileHandle findHandle = FindFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER + "*", filename, attr, FindFileFlags.ALL);
		if (!findHandle) {
			OBLLogger.Debug("No saved groups found");
			return;
		}
		if (filename != "" && filename.Contains(".json"))
			LoadGroup(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER + filename);
		while (FindNextFile(findHandle, filename, attr)) {
			if (filename != "" && filename.Contains(".json"))
				LoadGroup(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER + filename);
		}
		CloseFindFile(findHandle);
		OBLLogger.Debug("Loaded Groups: " + allGroups.Count());
	}
	
	void LoadGroup(string path) {
		if (!FileExist(path))
			return;
		OBLParty grp = new OBLParty();
		JsonFileLoader<OBLParty>.JsonLoadFile(path, grp);
		if (!grp)
			return;
		if (!grp.members)
			grp.members = new array<ref OBLPartyMember>();
		if (!grp.markers)
			grp.markers = new array<ref OBLMarker>();
		if (!grp.pings)
			grp.pings = new array<ref OBLMarker>();
		int inactivity = OBLPartyMainConfig.Get().inactiveGroupLifetimeDays * 3600 * 24;
		for (int i = 0; i < grp.members.Count(); i++) {
			OBLPartyMember mem = grp.members.Get(i);
			if (!mem) {
				grp.members.Remove(i);
				i--;
				continue;
			}
			OBLPartyPermission perm = mem.GetPermission();
			if (perm && perm.tempGroup) {
				grp.members.Remove(i);
				i--;
			}
		}
		if (inactivity > 0 || grp.members.Count() == 0) {
			int now = JMDate.Now(true).GetTimestamp();
			if (now - inactivity > grp.lastActivity || grp.members.Count() == 0) {
				OBLLogger.Debug("Deleted Group " + grp.shortname + " due to inactivity !");
				GetGame().AdminLog("Deleted Group " + grp.shortname + " due to inactivity !");
				DeleteGroup(grp);
				return;
			}
		}
		allGroups.Insert(grp);
		grp.OnLoadServer();
		OBLLogger.Debug("Loaded Group " + grp.shortname);
	}
	
	override OBLParty GetGroupByHash(int hash) {
		if (hash == 0)
			return null;
		foreach (OBLParty grp : allGroups) {
			if (!grp)
				continue;
			string shortName = grp.shortname + "";
			shortName.ToLower();
			int grpHash = shortName.Hash();
			if (grpHash == hash)
				return grp;
		}
		return null;
	}
	
	void DeleteGroup(string name) {
		if (name.Contains(".json"))
			name.Replace(".json", "");
		if (name.Length() == 0)
			return;
		string fromPath = OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPS_FOLDER + name + ".json";
		string toPath = OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPSDELETED_FOLDER + name + ".json";
		if (!FileExist(fromPath))
			return;
		int i = 0;
		while (FileExist(toPath))
			toPath = OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_GROUPSDELETED_FOLDER + name + "(" + (i++) + ").json";
		CopyFile(fromPath, toPath);
		DeleteFile(fromPath);
	}
	
	override void DeleteGroup(OBLParty group) {
		super.DeleteGroup(group);
		if (!group)
			return;
		string name = group.shortname;
		ref array<PlayerBase> onlineMembers = new array<PlayerBase>();
		if (group.playerChars) {
			foreach (PlayerBase member : group.playerChars) {
				if (member)
					onlineMembers.Insert(member);
			}
		}
		if (group.members) {
			foreach (OBLPartyMember partyMember : group.members) {
				if (partyMember && partyMember.steamid != "")
					cachedGroups.Set(partyMember.steamid, null);
			}
		}
		allGroups.RemoveItem(group);
		if (currentSaveIndex >= allGroups.Count())
			currentSaveIndex = allGroups.Count() - 1;
		DeleteGroup(name);
		foreach (PlayerBase onlineMember : onlineMembers) {
			if (onlineMember)
				onlineMember.ClearGroupServer(group);
		}
		if (group.playerChars)
			group.playerChars.Clear();
		delete group;
	}

}
