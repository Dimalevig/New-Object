class OBLAdminPage : OBLPartyPage {
	
	private ref array<ref OBLParty> groups = new array<ref OBLParty>();
	
	EditBoxWidget shortname, name, searchInput, maxPlayersInput;
	TextWidget level, created, lastactive, playercount, markercount, steamid, rank;
	ButtonWidget btnRefresh, btnDemote, btnPromote, btnKick, btnToLeader, btnCopy, btnJoin, btnDelete, btnChangeGroupNames, btnLevelUp, btnLevelDown, btnSetMaxPlayers;
	TextListboxWidget groupsList, memberList;
	CheckBoxWidget chckbxShortnames, chckbxNames, chckbxMemberNames, chckbxSteamids, chckbx_show_territorry_flags;
	MapWidget mapWidget;
	
	const string FLAG_POSITION_ICON = "OBL_SystemParty/gui/icons/flag.paa";
	const int FLAG_POSITION_COLOR = ARGB(255,255,255,255);
	ref array<ref Param2<ref vector, string>> flagPositions = new array<ref Param2<ref vector, string>>();
	
	override void StoreAllWidgetData(OBLDataSerializer data) {
		data.Write(new Param1<string>(searchInput.GetText()));
	}
	
	override void RestoreAllWidgetData(OBLDataSerializer data) {
		Param1<string> searchParam = Param1<string>.Cast(data.Read());
		searchInput.SetText(searchParam.param1);
	}
	
	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 6, 0, "Адмін", true);
	}
	
	override void OnShow() {
		super.OnShow();
		RequestGroups();
	}
	override void OnHide() {
		super.OnHide();
		ClearGroups();
	}
	
	override void InitMainWidget() {
		searchInput = EditBoxWidget.Cast(rootWidget.FindAnyWidget("searchInput"));
		shortname = EditBoxWidget.Cast(rootWidget.FindAnyWidget("shortname"));
		name = EditBoxWidget.Cast(rootWidget.FindAnyWidget("name"));
		
		level = TextWidget.Cast(rootWidget.FindAnyWidget("level"));
		created = TextWidget.Cast(rootWidget.FindAnyWidget("created"));
		lastactive = TextWidget.Cast(rootWidget.FindAnyWidget("lastactive"));
		playercount = TextWidget.Cast(rootWidget.FindAnyWidget("playercount"));
		markercount = TextWidget.Cast(rootWidget.FindAnyWidget("markercount"));
		steamid = TextWidget.Cast(rootWidget.FindAnyWidget("steamid"));
		rank = TextWidget.Cast(rootWidget.FindAnyWidget("rank"));
		
		btnRefresh = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnRefresh"));
		btnDemote = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnDemote"));
		btnPromote = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnPromote"));
		btnKick = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnKick"));
		btnToLeader = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnToLeader"));
		btnCopy = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnCopy"));
		btnJoin = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnJoin"));
		btnDelete = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnDelete"));
		btnChangeGroupNames = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnChangeGroupNames"));
		btnLevelUp = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnLevelUp"));
		btnLevelDown = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnLevelDown"));
		btnSetMaxPlayers = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnSetMaxPlayers"));
		maxPlayersInput = EditBoxWidget.Cast(rootWidget.FindAnyWidget("maxPlayersInput"));
		
		groupsList = TextListboxWidget.Cast(rootWidget.FindAnyWidget("groupsList"));
		memberList = TextListboxWidget.Cast(rootWidget.FindAnyWidget("memberList"));
		
		chckbxShortnames = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbxShortnames"));
		chckbxShortnames.SetChecked(true);
		chckbxNames = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbxNames"));
		chckbxNames.SetChecked(true);
		chckbxMemberNames = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbxMemberNames"));
		chckbxMemberNames.SetChecked(true);
		chckbxSteamids = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbxSteamids"));
		chckbxSteamids.SetChecked(true);
		chckbx_show_territorry_flags = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_show_territorry_flags"));
		chckbx_show_territorry_flags.SetChecked(false);
		chckbx_show_territorry_flags.Show(false);
		
		mapWidget = MapWidget.Cast(rootWidget.FindAnyWidget("mapWidget"));
	}
	
	void ShowTerritorryCheckbox() {
		chckbx_show_territorry_flags.Show(true);
	}
	
	void OnAllGroupsReceived(ParamsReadContext ctx) {
		OBLLogger.Debug("OnAllGroupsReceived");
		ClearGroups();
		if (!rootWidget.IsVisible())
			return;
		int count = 0;
		if (!ctx.Read(count))
			return;
		for (int i = 0; i < count; i++) {
			OBLParty grp = new OBLParty();
			bool exists = false;
			if (!ctx.Read(exists))
				return;
			if (!exists)
				continue;
			if (!grp.ReadFromCtx(ctx))
				return;
			groups.Insert(grp);
			grp.RemoveMarkersForAdminPage();
		}
		LoadAllGroups();
	}
	
	void ClearGroups() {
		foreach (OBLParty grp : groups) {
			delete grp;
		}
		groups.Clear();
	}
	
	void OnGroupUpdateReceived(ParamsReadContext ctx) {
		OBLLogger.Debug("OnGroupUpdateReceived");
		OBLParty grp = new OBLParty();
		string oldShortname;
		if (!ctx.Read(oldShortname))
			return;
		if (!grp.ReadFromCtx(ctx))
			return;
		// OBL FIX: without this the refreshed group's markers were drawn as 3D markers in the world
		grp.RemoveMarkersForAdminPage();
		bool found = false;
		for (int i = 0; i < groups.Count(); i++) {
			if (groups.Get(i).shortname == oldShortname) {
				groups.Set(i, grp);
				found = true;
				break;
			}
		}
		if (!found)
			groups.Insert(grp);
		LoadAllGroups();
	}
	
	void SetFlagPositions(array<ref Param2<ref vector, string>> arr) {
		flagPositions = arr;
		DisplayFlagPositions();
	}
	
	void DisplayFlagPositions() {
		if (!chckbx_show_territorry_flags.IsChecked())
			return;
		mapWidget.ClearUserMarks();
		foreach (Param2<ref vector, string> param : flagPositions) {
			mapWidget.AddUserMark(param.param1, param.param2, FLAG_POSITION_COLOR, FLAG_POSITION_ICON);
		}
	}
	
	void OnGroupDeleteReceived(ParamsReadContext ctx) {
		OBLLogger.Debug("OnGroupDeleteReceived");
		Param1<string> shortnameParam;
		if (!ctx.Read(shortnameParam))
			return;
		string shortname_ = shortnameParam.param1;
		for (int i = 0; i < groups.Count(); i++) {
			if (groups.Get(i).shortname == shortname_) {
				groups.RemoveOrdered(i);
				break;
			}
		}
		LoadAllGroups();
	}
	
	void LoadAllGroups() {
		OBLLogger.Debug("LoadAllGroups");
		FillGroupList();
		int row = groupsList.GetSelectedRow();
		if (row >= 0 && row < groupsList.GetNumItems()) {
			OBLParty grp2 = null;
			groupsList.GetItemData(row, 0, grp2);
			if (grp2)
				OnGroupSelected(grp2);
		}
	}
	
	void FillGroupList() {
		OBLLogger.Debug("FillGroupList");
		bool filter = searchInput.GetText().Length() > 0 && (chckbxShortnames.IsChecked() || chckbxNames.IsChecked() || chckbxMemberNames.IsChecked() || chckbxSteamids.IsChecked());

		int items = groupsList.GetNumItems();
		int i = 0;
		if (!filter) {
			foreach (OBLParty grp : groups) {
				if (i >= items)
					groupsList.AddItem(grp.shortname + " (" + grp.name + ")", grp, 0);
				else
					groupsList.SetItem(i, grp.shortname + " (" + grp.name + ")", grp, 0);
				i++;
			}
		} else {
			foreach (OBLParty grp2 : groups) {
				if (!IsSearched(grp2))
					continue;
				if (i >= items)
					groupsList.AddItem(grp2.shortname + " (" + grp2.name + ")", grp2, 0);
				else
					groupsList.SetItem(i, grp2.shortname + " (" + grp2.name + ")", grp2, 0);
				i++;
			}
		}
		while (i < items) {
			groupsList.RemoveRow(i);
			items--;
		}
		if (groupsList.GetSelectedRow() >= items)
			groupsList.SelectRow(items - 1);
	}
	
	bool IsSearched(OBLParty grp) {
		string searchStr = searchInput.GetText();
		searchStr.ToLower();
		if (chckbxNames.IsChecked()) {
			string lowerName = grp.name + "";
			lowerName.ToLower();
			if (lowerName.Contains(searchStr))
				return true;
		}
		if (chckbxShortnames.IsChecked()) {
			string shortLower = grp.shortname + "";
			shortLower.ToLower();
			if (shortLower.Contains(searchStr))
				return true;
		}
		if (chckbxMemberNames.IsChecked()) {
			foreach (OBLPartyMember member : grp.members) {
				string memberLower = member.name + "";
				memberLower.ToLower();
				if (memberLower.Contains(searchStr))
					return true;
			}
		}
		if (chckbxSteamids.IsChecked()) {
			foreach (OBLPartyMember member2 : grp.members) {
				string steamidLower = member2.steamid + "";
				steamidLower.ToLower();
				if (steamidLower.Contains(searchStr))
					return true;
			}
		}
		return false;
	}
	
	void OnGroupSelected(OBLParty grp) {
		shortname.SetText(grp.shortname);
		name.SetText(grp.name);
		level.SetText("" + grp.level);
		created.SetText(JMDate.Epoch(grp.creationDate).ToString("DD.MM.YYYY hh:mm:ss"));
		lastactive.SetText(JMDate.Epoch(grp.lastActivity).ToString("DD.MM.YYYY hh:mm:ss"));
		playercount.SetText("" + grp.members.Count() + "/" + grp.maxPlayers);
		if (maxPlayersInput)
			maxPlayersInput.SetText("" + grp.maxPlayers);
		markercount.SetText("" + grp.markers.Count() + "/" + grp.markerLimit);
		
		FillMemberList(grp);
		chckbx_show_territorry_flags.SetChecked(false);
		mapWidget.ClearUserMarks();
		foreach (OBLMarker marker : grp.markers) {
			mapWidget.AddUserMark(marker.position, marker.name, marker.GetColorARGB(), marker.icon);
		}
	}
	
	void FillMemberList(OBLParty grp) {
		int items = memberList.GetNumItems();
		int i = 0;
		foreach (OBLPartyMember member : grp.members) {
			if (i >= items)
				memberList.AddItem(member.name, member, 0);
			else
				memberList.SetItem(i, member.name, member, 0);
			i++;
		}
		
		while (i < items) {
			memberList.RemoveRow(i);
			items--;
		}
		if (memberList.GetSelectedRow() >= items)
			memberList.SelectRow(items - 1);
		
		int row = memberList.GetSelectedRow();
		if (row >= 0 && row < memberList.GetNumItems()) {
			OBLPartyMember member2 = null;
			memberList.GetItemData(row, 0, member2);
			if (member2)
				OnMemberSelected(member2);
		}
	}
	
	void ChangeGroupName(OBLParty grp) {
		string newName = name.GetText();
		string newShortname = shortname.GetText();
		if (newName == grp.name && newShortname == grp.shortname)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(grp.shortname);
		rpc.Write(newShortname);
		rpc.Write(newName);
		rpc.Send(null, OBLPartyRPCs.GROUP_ADMIN_NAMES, true);
	}
	
	void OnMemberSelected(OBLPartyMember member) {
		steamid.SetText(member.steamid);
		OBLPartyPermission perm = OBLPartyPermissions.Get().FindPermissionGroupByUID(member.permissionGroup);
		if (perm)
			rank.SetText(perm.permName);
		else
			rank.SetText("?");
	}
	
	// 0 = стандартний ліміт (6), 1..15 — свій ліміт для групи
	void SetGroupMaxPlayers(OBLParty grp) {
		if (!maxPlayersInput)
			return;
		string txt = maxPlayersInput.GetText();
		txt.Replace(" ", "");
		if (txt.Length() == 0)
			return;
		int newMax = txt.ToInt();
		if (newMax < 0 || newMax > OBLPartyConstants.MAX_PLAYERS_LIMIT || (newMax == 0 && txt != "0")) {
			NotificationSystem.AddNotificationExtended(4, "Система груп", "Вкажіть число від 1 до " + OBLPartyConstants.MAX_PLAYERS_LIMIT + " (0 — стандартний ліміт " + OBLPartyConstants.DEFAULT_MAX_PLAYERS + ")", "set:ccgui_enforce image:MapDestroyed");
			return;
		}
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(grp.shortname);
		rpc.Write(newMax);
		rpc.Send(null, OBLPartyRPCs.GROUP_ADMIN_MAX_PLAYERS, true);
	}
	
	void SetGroupLevel(OBLParty grp, int level_) {
		if (level_ < 0)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(grp.shortname);
		rpc.Write(level_);
		rpc.Send(null, OBLPartyRPCs.GROUP_ADMIN_LEVEL, true);
	}
	
	override bool OnItemSelected(Widget w, int row, int column) {
		if (super.OnItemSelected(w, row, column))
			return true;
		if (w == groupsList) {
			if (row < 0 || row >= groupsList.GetNumItems())
				return true;
			OBLParty grp;
			groupsList.GetItemData(row, 0, grp);
			if (grp)
				OnGroupSelected(grp);
			return true;
		} else if (w == memberList) {
			if (row < 0 || row >= memberList.GetNumItems())
				return true;
			OBLPartyMember member;
			memberList.GetItemData(row, 0, member);
			if (member)
				OnMemberSelected(member);
			return true;
		}
		return false;
	}
	
	override bool OnChange(Widget w) {
		if (w == searchInput) {
			FillGroupList();
			groupsList.SelectRow(0);
			groupsList.EnsureVisible(0);
			return true;
		}
		return false;
	}
	
	override bool OnClick(Widget w) {
		if (super.OnClick(w))
			return true;
		OBLParty grp;
		if (w == chckbxNames || w == chckbxSteamids || w == chckbxShortnames || w == chckbxMemberNames) {
			FillGroupList();
			return true;
		} else if (w == btnChangeGroupNames) {
			if (GetSelectedGroup(grp))
				ChangeGroupName(grp);
			return true;
		} else if (w == btnLevelDown) {
			if (GetSelectedGroup(grp))
				SetGroupLevel(grp, grp.level - 1);
			return true;
		} else if (w == btnLevelUp) {
			if (GetSelectedGroup(grp))
				SetGroupLevel(grp, grp.level + 1);
			return true;
		} else if (w == btnSetMaxPlayers) {
			if (GetSelectedGroup(grp))
				SetGroupMaxPlayers(grp);
			return true;
		} else if (w == btnRefresh) {
			RequestGroups();
			return true;
		} else if (w == btnJoin) {
			ScriptRPC rpc = new ScriptRPC();
			if (GetSelectedGroup(grp)) {
				rpc.Write(grp.shortname);
			} else {
				return true;
			}
			rpc.Send(null, OBLPartyRPCs.GROUP_ADMIN_JOIN, true);
			return true;
		} else if (w == btnDelete) {
			rpc = new ScriptRPC();
			if (GetSelectedGroup(grp)) {
				rpc.Write(grp.shortname);
			} else {
				return true;
			}
			rpc.Send(null, OBLPartyRPCs.GROUP_ADMIN_DELETE, true);
			return true;
		} else if (w == btnKick) {
			SendSelectedGroupMemberRPC(OBLPartyRPCs.GROUP_ADMIN_KICK);
		} else if (w == btnDemote) {
			SendSelectedGroupMemberRPC(OBLPartyRPCs.GROUP_ADMIN_DEMOTE);
		} else if (w == btnPromote) {
			SendSelectedGroupMemberRPC(OBLPartyRPCs.GROUP_ADMIN_PROMOTE);
		} else if (w == btnToLeader) {
			SendSelectedGroupMemberRPC(OBLPartyRPCs.GROUP_ADMIN_TOLEADER);
		} else if (w == btnCopy) {
			CopySelectedUserData();
		} else if (w == chckbx_show_territorry_flags) {
			DisplayFlagPositions();
		}
		return false;
	}
	
	void CopySelectedUserData() {
		ScriptRPC rpc = new ScriptRPC();
		OBLParty grp;
		if (!GetSelectedGroup(grp))
			return;
		int row = memberList.GetSelectedRow();
		if (row < 0 || row >= memberList.GetNumItems())
			return;
		OBLPartyMember member;
		memberList.GetItemData(row, 0, member);
		if (!member)
			return;
		string cop = member.steamid + " " + member.name;
		GetGame().CopyToClipboard(cop);
		NotificationSystem.AddNotificationExtended(4, "Система груп", "Дані користувача скопійовано", "set:ccgui_enforce image:HudUserMarker");
	}
	
	void SendSelectedGroupMemberRPC(int rpc_type) {
		ScriptRPC rpc = new ScriptRPC();
		OBLParty grp;
		string steamid_;
		if (!GetSelectedGroup(grp) || !GetSelectedPlayerSteamId(steamid_))
			return;
		string shortname_ = grp.shortname;
		rpc.Write(shortname_);
		rpc.Write(steamid_);
		rpc.Send(null, rpc_type, true);
	}
	
	bool GetSelectedPlayerSteamId(out string steamid_) {
		int row = memberList.GetSelectedRow();
		if (row < 0 || row >= memberList.GetNumItems())
			return false;
		OBLPartyMember member;
		memberList.GetItemData(row, 0, member);
		if (!member)
			return false;
		steamid_ = member.steamid;
		return true;
	}
	
	bool GetSelectedGroup(out OBLParty grp) {
		int row = groupsList.GetSelectedRow();
		if (row < 0 || row >= groupsList.GetNumItems())
			return false;
		groupsList.GetItemData(row, 0, grp);
		if (grp)
			return true;
		return false;
	}
	
	void RequestGroups() {
		GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_ADMIN_LIST, new Param1<bool>(true), true);
		GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_ADMIN_FLAGS, new Param1<bool>(true), true);
	}
	
}
cla