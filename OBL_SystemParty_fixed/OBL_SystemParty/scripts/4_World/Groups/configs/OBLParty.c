class OBLParty {

	static int CONFIG_VERSION = 1;
	int configVersion = 0;
	string name, shortname;
	int level;
	int maxPlayers;
	int subGroupSize;
	int subGroupCount;
	int lastActivity = -1;
	int creationDate = -1;
	int markerLimit = 0;
	int plotpoleLimit = 0;
	bool showTagInChat = true;
	// 0 = стандартний ліміт (6); 1..15 — ліміт, заданий адміном (можна вписати і в JSON групи)
	int maxPlayersOverride = 0;
	ref array<ref OBLPartyMember> members = new array<ref OBLPartyMember>();
	ref array<ref OBLMarker> markers = new array<ref OBLMarker>();
	[NonSerialized()]
	ref array<ref OBLMarker> pings = new array<ref OBLMarker>();
	
	void OnLoadServer() {
		if (configVersion != CONFIG_VERSION) {
			if (configVersion < 1) {
				showTagInChat = true;
			}
		}
		configVersion = CONFIG_VERSION;
	}
	
	void ~OBLParty() {
		if (!GetGame() || GetGame().IsServer())
			return;
		while (members && members.Count() > 0) {
			OBLPartyMember member = members.Get(0);
			members.Remove(0);
			if (member)
				delete member;
		}
		while (markers && markers.Count() > 0) {
			OBLMarker marker = markers.Get(0);
			markers.Remove(0);
			if (marker)
				delete marker;
		}
		while (pings && pings.Count() > 0) {
			OBLMarker ping = pings.Get(0);
			pings.Remove(0);
			if (ping)
				delete ping;
		}
	}
	
	void InitMarkers() {
		foreach (OBLMarker marker : markers) {
			if (marker)
				marker.InitMarker();
		}
		foreach (OBLPartyMember member : members) {
			if (member)
				member.InitMarker();
		}
		foreach (OBLMarker ping : pings) {
			if (ping)
				ping.InitMarker();
		}
	}
	
	void RemoveMarkersForAdminPage() {
		foreach (OBLMarker marker : markers) {
			if (marker)
				marker.RemoveFromAllList();
		}
		foreach (OBLPartyMember member : members) {
			if (member)
				member.RemoveFromAllList();
		}
		foreach (OBLMarker ping : pings) {
			if (ping)
				ping.RemoveFromAllList();
		}
	}
	
	void OnRPCClient(int type, ParamsReadContext ctx) {
		if (type == OBLPartyRPCs.ADD) {
			OBLMarker marker = new OBLMarker();
			if (!marker.ReadFromCtx(ctx)) {
				OBLLogger.Warn("Failed to receive new Marker from Server !");
				return;
			}
			AddMarkerLocal(marker);
		} else if (type == OBLPartyRPCs.REMOVE) {
			int uid;
			if (!ctx.Read(uid)) {
				OBLLogger.Warn("Failed to receive new Marker ID from Server !");
				return;
			}
			OBLMarker mark = FindAnyMarkerByUID(uid);
			if (mark) {
				RemoveMarkerLocal(mark);
			}
		} else if (type == OBLPartyRPCs.CHANGE_TAG_VISIBILITY) {
			bool enabled;
			if (!ctx.Read(enabled)) {
				OBLLogger.Warn("Failed to receive Tag Visibility from Server !");
				return;
			}
			showTagInChat = enabled;
		} else if (type == OBLPartyRPCs.ADD_CLIENT) {
			OBLPartyMember member = new OBLPartyMember();
			if (!member.ReadFromCtx(ctx)) {
				OBLLogger.Warn("Failed to receive new Member from Server !");
				return;
			}
			AddMarkerLocal(member);
		} else if (type == OBLPartyRPCs.UPGRADE) {
			int level1, maxPlayers1, subGroupCount1, subGroupSize1, maxMarkers1, plotpoleLimit1;
			if (!ctx.Read(level1) || !ctx.Read(maxPlayers1) || !ctx.Read(subGroupCount1) || !ctx.Read(subGroupSize1) || !ctx.Read(maxMarkers1) || !ctx.Read(plotpoleLimit1)) {
				OBLLogger.Warn("Failed to Read Upgrade Info");
				return;
			}
			level = level1;
			maxPlayers = maxPlayers1;
			subGroupCount = subGroupCount1;
			subGroupSize = subGroupSize1;
			markerLimit = maxMarkers1;
			plotpoleLimit = plotpoleLimit1;
			MissionBaseWorld mission = MissionBaseWorld.Cast(GetGame().GetMission());
			if (mission)
				mission.OnGroupChanged();
		} else if (type > OBLPartyRPCs.START_MARKER_RPC) {
			int uid2;
			if (!ctx.Read(uid2))
				return;
			OBLMarker marker2 = FindAnyMarkerByUID(uid2);
			if (marker2)
				marker2.OnMarkerRPCClient(type, ctx);
		}
	}
	
	void KickPlayerClient(string steamid) {
		PromoteDemoteKickHelpter(steamid, OBLPartyRPCs.KICK);
	}
	
	void DemotePlayerClient(string steamid) {
		PromoteDemoteKickHelpter(steamid, OBLPartyRPCs.DEMOTE);
	}
	
	void PromotePlayerClient(string steamid) {
		PromoteDemoteKickHelpter(steamid, OBLPartyRPCs.PROMOTE);
	}
	
	void PromoteDemoteKickHelpter(string steamid, int type) {
		ScriptRPC rpc = CreateRPCCall(type);
		rpc.Write(steamid);
		SendRPCToServer(rpc);
	}
	
	void LeaveGroupClient() {
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.LEAVE);
		SendRPCToServer(rpc);
	}
	
	OBLPartyMember GetMemberBySteamid(string steamid) {
		foreach (OBLPartyMember member : members) {
			if (member && member.steamid == steamid)
				return member;
		}
		return null;
	}
	
	bool IsMember(string steamid) {
		return GetMemberBySteamid(steamid) != null;
	}
	
	void RemoveMember(OBLPartyMember member) {
		if (!member)
			return;
		int uidd = member.uid;
		for (int i = 0; i < members.Count(); i++) {
			OBLPartyMember mem = members.Get(i);
			if (!mem || mem.uid == uidd) {
				members.Remove(i);
				i--;
				if (mem)
					delete mem;
			}
		}
	}
	
	int GetSubgroupMemberCount(int subGroup) {
		return GetSubgroupMembers(subGroup).Count();
	}
	
	array<ref OBLPartyMember> GetSubgroupMembers(int subGroup) {
		if (!OBLPartyMainConfig.Get().enableSubGroups)
			return members;
		array<ref OBLPartyMember> memb = new array<ref OBLPartyMember>();
		foreach (OBLPartyMember member : members) {
			if (member && member.currentSubgroup == subGroup)
				memb.Insert(member);
		}
		return memb;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(name);
		ctx.Write(shortname);
		ctx.Write(members.Count());
		foreach (OBLPartyMember member : members) {
			member.WriteToCtx(ctx);
		}
		ctx.Write(markers.Count());
		foreach (OBLMarker marker : markers) {
			marker.WriteToCtx(ctx);
		}
		ctx.Write(level);
		ctx.Write(maxPlayers);
		ctx.Write(subGroupSize);
		ctx.Write(subGroupCount);
		ctx.Write(markerLimit);
		ctx.Write(plotpoleLimit);
		ctx.Write(showTagInChat);
		ctx.Write(creationDate);
		ctx.Write(lastActivity);
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(name))
			return false;
		if (!ctx.Read(shortname))
			return false;
		int count = 0;
		if (!ctx.Read(count))
			return false;
		for (int i = 0; i < count; i++) {
			OBLPartyMember member = new OBLPartyMember();
			if (!member.ReadFromCtx(ctx))
				return false;
			members.Insert(member);
		}
		count = 0;
		if (!ctx.Read(count))
			return false;
		for (i = 0; i < count; i++) {
			OBLMarker marker = new OBLMarker();
			if (!marker.ReadFromCtx(ctx))
				return false;
			markers.Insert(marker);
		}
		if (!ctx.Read(level))
			return false;
		if (!ctx.Read(maxPlayers))
			return false;
		if (!ctx.Read(subGroupSize))
			return false;
		if (!ctx.Read(subGroupCount))
			return false;
		if (!ctx.Read(markerLimit))
			return false;
		if (!ctx.Read(plotpoleLimit))
			return false;
		if (!ctx.Read(showTagInChat))
			return false;
		if (!ctx.Read(creationDate))
			return false;
		if (!ctx.Read(lastActivity))
			return false;
		return true;
	}
	
	void AddMarker(OBLMarker marker) {
		if (markers.Count() >= markerLimit && marker.type != OBLMarkerType.GROUP_PING) {
			SendErrorNotificationLOCAL("Досягнуто ліміт маркерів!");
			return;
		}
		OBLLogger.Debug("Sending Add Marker request to Server ...");
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.ADD);
		marker.WriteToCtx(rpc);
		SendRPCToServer(rpc);
	}
	
	void SendErrorNotificationLOCAL(string message, float show_time = 4) {
		NotificationSystem.AddNotificationExtended(show_time, OBLTheme.NOTIFY_TITLE, message, OBLTheme.ICON_ERROR);
	}
	
	void SendInfoNotificationLOCAL(string message, float show_time = 4) {
		NotificationSystem.AddNotificationExtended(show_time, OBLTheme.NOTIFY_TITLE, message, OBLTheme.ICON_INFO);
	}
	
	void RemoveMarker(OBLMarker marker) {
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.REMOVE);
		rpc.Write(marker.uid);
		SendRPCToServer(rpc);
	}
	
	void SendRPCToServer(ScriptRPC rpc) {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb)
			return;
		rpc.Send(pb, OBLPartyRPCs.GROUP_RPC, true);
	}
	
	ScriptRPC CreateRPCCall(int type) {
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(type);
		return rpc;
	}
	
	void SendChatTagVisibilityRequest(bool visible) {
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.CHANGE_TAG_VISIBILITY);
		rpc.Write(visible);
		SendRPCToServer(rpc);
	}
	
	void SendPlayerInviteClient(string steamid) {
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.INVITE);
		rpc.Write(steamid);
		SendRPCToServer(rpc);
	}
	
	void AddMarkerLocal(OBLMarker marker) {
		if (marker.type == OBLMarkerType.GROUP_PING) {
			pings.Insert(marker);
			marker.InitMarker();
		} else if (marker.type == OBLMarkerType.GROUP_MARKER) {
			markers.Insert(marker);
			marker.InitMarker();
		} else if (marker.type == OBLMarkerType.GROUP_PLAYER_MARKER) {
			OBLPartyMember member = OBLPartyMember.Cast(marker);
			if (!member)
				return;
			members.Insert(member);
			member.parentGroup = this;
			member.InitMarker();
		} else {
			OBLLogger.Warn("Trying to add Marker to Group which is no Group Marker type. Type Got: " + marker.type);
		}
		
	}
	
	void RemoveMarkerLocal(OBLMarker marker) {
		if (!marker)
			return;
		if (marker.type == OBLMarkerType.GROUP_PING) {
			for (int i = 0; i < pings.Count(); i++) {
				OBLMarker mark = pings.Get(i);
				if (mark && mark.uid == marker.uid) {
					pings.Remove(i);
					i--;
					delete mark;
					if (!marker)
						break;
				}
			}
		} else if (marker.type == OBLMarkerType.GROUP_MARKER) {
			for (i = 0; i < markers.Count(); i++) {
				mark = markers.Get(i);
				if (mark && mark.uid == marker.uid) {
					markers.Remove(i);
					i--;
					delete mark;
					if (!marker)
						break;
				}
			}
		} else if (marker.type == OBLMarkerType.GROUP_PLAYER_MARKER) {
			OBLPartyMember member = OBLPartyMember.Cast(marker);
			if (!member)
				return;
			RemoveMember(member);
		} else {
			OBLLogger.Warn("Trying to remove Marker form Group which is no Group Marker type. Type Got: " + marker.type);
		}
	}
	
	OBLMarker FindMarkerByUID(int uid) {
		foreach (OBLMarker marker : markers) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}
	
	OBLMarker FindPingMarkerByUID(int uid) {
		foreach (OBLMarker marker : pings) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}
	
	OBLPartyMember FindMemberByUID(int uid) {
		foreach (OBLPartyMember member : members) {
			if (member && member.uid == uid)
				return member;
		}
		return null;
	}
	
	OBLMarker FindAnyMarkerByUID(int uid) {
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Finding any Marker with UID: " + uid + " Markers: " + markers.Count() + " Members: " + members.Count() + " Pings: " + pings.Count());
		OBLMarker marker = FindMarkerByUID(uid);
		if (!marker)
			marker = FindMemberByUID(uid);
		if (!marker)
			marker = FindPingMarkerByUID(uid);
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Found Marker: " + marker);
		return marker;
	}
	
	bool FindNearestMarker(vector position, out OBLMarker markero, out float distance) {
		if (markers.Count() == 0)
			return false;
		float bestDist = 0;
		OBLMarker bestMarker = null;
		foreach (OBLMarker marker : markers) {
			if (!marker)
				continue;
			float markerDist = vector.Distance(position, marker.position);
			if (!bestMarker || markerDist < bestDist) {
				bestMarker = marker;
				bestDist = markerDist;
			}
		}
		if (bestMarker) {
			markero = bestMarker;
			distance = bestDist;
			return true;
		}
		return false;
	}
	
	void SendRPCToGroupMembers(ScriptRPC rpc, int otherRPCType = -1) {}
	void InitNumbers() {}
	// реалізовано в серверному моді
	void OnInviteActionServer(PlayerBase inviter, PlayerBase target) {}
}
