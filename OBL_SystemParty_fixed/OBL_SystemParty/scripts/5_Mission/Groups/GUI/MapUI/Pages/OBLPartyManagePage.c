class OBLPartyManagePage : OBLPartyPage {
	
	EditBoxWidget searchbox;
	ButtonWidget buttonInvite;
	ButtonWidget btn_kick, btn_leave, btn_promote, btn_demote, btn_joinSubgroup;
	TextWidget txt_groupname;
	TextListboxWidget playerlist_online, playerlist_members;

	void OBLPartyManagePage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);
	}
	
	void ~OBLPartyManagePage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
	}
	
	void OnStreamerModeChange(bool enabled) {
		if (playerlist_online)
			playerlist_online.Show(!enabled);
		if (txt_groupname)
			txt_groupname.Show(!enabled);
	}
	
	override bool InitPage(OBLPartyUI parentUI) {
		bool worked = super.InitPage(parentUI, 1, 1, "Група", false);
		if (buttonWidget)
			parentUI.groupButton = buttonWidget;
		return worked;
	}
	
	override bool IsAvailable() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		return pb && pb.GetOBLParty() != null;
	}
	
	override void StoreAllWidgetData(OBLDataSerializer data) {
		data.Write(new Param1<string>(searchbox.GetText()));
	}
	
	override void RestoreAllWidgetData(OBLDataSerializer data) {
		Param1<string> searchParam = Param1<string>.Cast(data.Read());
		searchbox.SetText(searchParam.param1);
	}
	
	override bool OnTopButtonClicked(Widget w) {
		return super.OnTopButtonClicked(w);
	}
	
	override void OnShow() {
		OBLLogger.Debug("GroupManagePage OnShow");
		super.OnShow();
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		OBLParty grp;
		if (pb)
			grp = pb.GetOBLParty();
		if (!grp) {
			if (txt_groupname)
				txt_groupname.SetText("");
			ClearMembersList();
			UpdateInviteButton();
			return;
		}
		if (txt_groupname)
			txt_groupname.SetText(grp.name + " (" + grp.members.Count() + "/" + grp.maxPlayers + ")");
		FillOnlinePlayersList();
		FillMembersList();
		UpdateInviteButton();
		OnStreamerModeChange(OBLLayoutConfig.Get().streamerModeEnabled);

	}
	
	override void OnUpdateSlow() {
		OBLLogger.Debug("GroupManagePage OnUpdateSlow");
		PlayerBase pbSlow = PlayerBase.Cast(GetGame().GetPlayer());
		if (txt_groupname && pbSlow && pbSlow.GetOBLParty())
			txt_groupname.SetText(pbSlow.GetOBLParty().name + " (" + pbSlow.GetOBLParty().members.Count() + "/" + pbSlow.GetOBLParty().maxPlayers + ")");
		FillOnlinePlayersList();
		FillMembersList();
		UpdateInviteButton();
	}
	
	override bool OnClick(Widget w) {
		PlayerBase pb;
		if (w == buttonInvite) {
			string inviteTarget = GetSelectedInviteTarget();
			if (inviteTarget == "")
				inviteTarget = GetTypedInviteCode();
			if (inviteTarget == "")
				return true;
			pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pb || !pb.GetOBLParty())
				return true;
			pb.GetOBLParty().SendPlayerInviteClient(inviteTarget);
			return true;
		} else if (w == btn_leave) {
			pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pb || !pb.GetOBLParty())
				return true;
			pb.GetOBLParty().LeaveGroupClient();
			return true;
		} else if (w == btn_promote) {
			PromoteSelectedPlayer();
			return true;
		} else if (w == btn_demote) {
			DemoteSelectedPlayer();
			return true;
		} else if (w == btn_kick) {
			KickSelectedPlayer();
			return true;
		}
		return false;
	}
	
	override bool OnChange(Widget w) {
		if (w == searchbox) {
			FillOnlinePlayersList();
			UpdateInviteButton();
			return true;
		}
		return false;
	}
	override bool OnItemSelected(Widget w, int row, int column) {
		if (w == playerlist_online) {
			UpdateInviteButton();
			return true;
		} else if (w == playerlist_members) {
			UpdateGroupButtons();
			return true;
		}
		return false;
	}
	void UpdateInviteButton() {
		if (!buttonInvite)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty()) {
			buttonInvite.Enable(false);
			return;
		}
		OBLPartyPermission myPerm = pb.GetPermission();
		if (!myPerm || !myPerm.canInvite) {
			buttonInvite.Enable(false);
			return;
		}
		string selectedTarget = GetSelectedInviteTarget();
		if (selectedTarget == "") {
			buttonInvite.Enable(GetTypedInviteCode() != "");
			return;
		}
		foreach (OBLPartyMember member : pb.GetOBLParty().members) {
			if (member && member.steamid == selectedTarget) {
				buttonInvite.Enable(false);
				return;
			}
		}
		buttonInvite.Enable(true);
	}

	string GetSelectedInviteTarget() {
		int row = playerlist_online.GetSelectedRow();
		if (row < 0 || row >= playerlist_online.GetNumItems())
			return "";
		Param1<string> steamIdParam;
		playerlist_online.GetItemData(row, 0, steamIdParam);
		if (!steamIdParam)
			return "";
		return steamIdParam.param1;
	}

	string GetTypedInviteCode() {
		if (!searchbox)
			return "";
		string inviteCode = searchbox.GetText();
		inviteCode.Replace(" ", "");
		if (!IsInviteCode(inviteCode))
			return "";
		return inviteCode;
	}

	bool IsInviteCode(string inviteCode) {
		int length = inviteCode.Length();
		if (length < 4 || length > 6)
			return false;
		for (int i = 0; i < length; i++) {
			if ("0123456789".IndexOf(inviteCode[i]) == -1)
				return false;
		}
		return true;
	}
	
	void UpdateGroupButtons() {
		MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!mission || !pb || !pb.GetOBLParty())
			return;
		string steamid = mission.mySteamid;
		OBLPartyPermission myPerms = pb.GetPermission();
		if (!myPerms)
			return;
		int selected = playerlist_members.GetSelectedRow();
		if (selected < 0 || selected >= playerlist_members.GetNumItems()) {
			btn_kick.Enable(false);
			btn_promote.Enable(false);
			btn_demote.Enable(false);
		} else {
			Param3<string, ref OBLPartyPermission, int> selectedPerm;
			playerlist_members.GetItemData(selected, 0, selectedPerm);
			if (!selectedPerm) {
				btn_kick.Enable(false);
				btn_promote.Enable(false);
				btn_demote.Enable(false);
				return;
			}
			if (selectedPerm.param1.Length() != 17) {
				// рядок-заголовок підгрупи «Онлайн»/«Офлайн»
				btn_kick.Enable(false);
				btn_promote.Enable(false);
				btn_demote.Enable(false);
			} else if (selectedPerm.param1 == steamid) {
				btn_kick.Enable(false);
				btn_promote.Enable(false);
				btn_demote.Enable(false);
			} else {
				btn_kick.Enable(myPerms.CanKick(selectedPerm.param2));
				btn_promote.Enable(myPerms.CanPromote(selectedPerm.param2));
				btn_demote.Enable(myPerms.CanDemote(selectedPerm.param2));
			}
		}
	}
	
	override bool OnDoubleClick(Widget w) {
		return false;
	}
	
	override void OnGroupChanged() {
		
	}
	
	void FillOnlinePlayersList() {
		OBLLogger.Debug("Filling Online Players List...");
		if (!ClientData.m_PlayerList || !ClientData.m_PlayerList.m_PlayerList)
			return;
		array<ref SyncPlayer> list = ClientData.m_PlayerList.m_PlayerList;
		int items = playerlist_online.GetNumItems();
		int added = 0;
		for (int i = 0; i < list.Count(); i++) {
			SyncPlayer player = list.Get(i);
			string name = player.m_PlayerName;
			string steamid = player.m_UID;
			if (OBLOnlinePrivacyManager.IsHidden(steamid))
				continue;
			Param1<string> param = new Param1<string>(steamid);
			if (IsSearched(name)) {
				if (items <= added) {
					playerlist_online.AddItem(" " + name, param, 0);
				} else {
					playerlist_online.SetItem(added, " " + name, param, 0);
				}
				added++;
			}
		}
		for (i = added; i < items; i++) {
			playerlist_online.RemoveRow(added);
		}
	}
	
	bool IsSearched(string name) {
		string lowerName = name + "";
		lowerName.ToLower();
		string lowerSearch = searchbox.GetText();
		lowerSearch.ToLower();
		if (lowerSearch.Length() == 0)
			return true;
		return lowerName.IndexOf(lowerSearch) != -1;
	}
	
	void FillMembersList() {
		OBLLogger.Debug("FillMembersList");
		
		int items = playerlist_members.GetNumItems();
		int added = 0;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty()) {
			ClearMembersList();
			return;
		}
		OBLParty grp = pb.GetOBLParty();
		int lastSubgroup = 0;
		int subgroupMaxSize = grp.subGroupSize;
		array<ref OBLPartyMember> sortedMembers = SortGroupMembers(grp);
		foreach (OBLPartyMember member : sortedMembers) {
			if (!member)
				continue;
			string addName = "";
			Param3<string, ref OBLPartyPermission, int> param;
			if (OBLPartyMainConfig.Get().enableSubGroups) {
				while (member.currentSubgroup >= lastSubgroup && lastSubgroup < grp.subGroupCount) {
					param = new Param3<string, ref OBLPartyPermission, int>("", null, lastSubgroup);
					int inCount = grp.GetSubgroupMemberCount(lastSubgroup);
					if (items <= added) {
						playerlist_members.AddItem(" " + OBLPartyMainConfig.Get().GetSubGroupName(lastSubgroup) + " (" + inCount + "/" + subgroupMaxSize + ")", param, 0);
					} else {
						playerlist_members.SetItem(added, " " + OBLPartyMainConfig.Get().GetSubGroupName(lastSubgroup) + " (" + inCount + "/" + subgroupMaxSize + ")", param, 0);
					}
					playerlist_members.SetItemColor(added, 0, OBLTheme.Muted());
					added++;
					lastSubgroup++;
				}
				addName = "  ";
			}
			string steamid = member.steamid;
			OBLPartyPermission perm = OBLPartyPermissions.Get().FindPermissionGroupByUID(member.permissionGroup);
			if (!perm)
				continue;
			string groupname = perm.permName;
			string displayname = addName + member.name + " (" + groupname + ")";
			param = new Param3<string, ref OBLPartyPermission, int>(steamid, perm, -1);
			if (items <= added) {
				playerlist_members.AddItem(" " + displayname, param, 0);
			} else {
				playerlist_members.SetItem(added, " " + displayname, param, 0);
			}
			playerlist_members.SetItemColor(added, 0, member.GetColorARGB());
			added++;
		}
		if (OBLPartyMainConfig.Get().enableSubGroups) {
			while (grp.subGroupCount > lastSubgroup) {
				param = new Param3<string, ref OBLPartyPermission, int>("", null, lastSubgroup);
				if (items <= added) {
					playerlist_members.AddItem(" " + OBLPartyMainConfig.Get().GetSubGroupName(lastSubgroup) + " (0/" + subgroupMaxSize + ")", param, 0);
				} else {
					playerlist_members.SetItem(added, " " + OBLPartyMainConfig.Get().GetSubGroupName(lastSubgroup) + " (0/" + subgroupMaxSize + ")", param, 0);
				}
				playerlist_members.SetItemColor(added, 0, OBLTheme.Muted());
				added++;
				lastSubgroup++;
			}
		}
		for (int i = added; i < items; i++) {
			playerlist_members.RemoveRow(added);
		}
	}
	
	void ClearMembersList() {
		if (!playerlist_members)
			return;
		while (playerlist_members.GetNumItems() > 0) {
			playerlist_members.RemoveRow(0);
		}
	}
	
	array<ref OBLPartyMember> SortGroupMembers(OBLParty grp) {
		if (!OBLPartyMainConfig.Get().enableSubGroups)
			return grp.members;
		array<ref OBLPartyMember> mem = new array<ref OBLPartyMember>();
		for (int i = 0; i < grp.subGroupCount; i++) {
			array<ref OBLPartyMember> subgrpMembers = grp.GetSubgroupMembers(i);
			foreach (OBLPartyMember member : subgrpMembers)
				mem.Insert(member);
		}
		// OBL FIX: members in a subgroup outside the range used to vanish from the list
		foreach (OBLPartyMember rest : grp.members) {
			if (rest && mem.Find(rest) == -1)
				mem.Insert(rest);
		}
		return mem;
	}
	
	void PromoteSelectedPlayer() {
		string steamid = GetSelectedPlayerSteamid();
		if (steamid.Length() != 17)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		pb.GetOBLParty().PromotePlayerClient(steamid);
	}
	
	void DemoteSelectedPlayer() {
		string steamid = GetSelectedPlayerSteamid();
		if (steamid.Length() != 17)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		pb.GetOBLParty().DemotePlayerClient(steamid);
	}
	
	void KickSelectedPlayer() {
		string steamid = GetSelectedPlayerSteamid();
		if (steamid.Length() != 17)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		pb.GetOBLParty().KickPlayerClient(steamid);
	}
	
	string GetSelectedPlayerSteamid() {
		int row = playerlist_members.GetSelectedRow();
		if (row < 0 || row >= playerlist_members.GetNumItems())
			return "";
		Param3<string, ref OBLPartyPermission, int> selectedPerm;
		playerlist_members.GetItemData(row, 0, selectedPerm);
		if (selectedPerm)
			return selectedPerm.param1;
		return "";
	}
	
	int GetSelectedSubGroup() {
		int row = playerlist_members.GetSelectedRow();
		if (row < 0 || row >= playerlist_members.GetNumItems())
			return -1;
		Param3<string, ref OBLPartyPermission, int> selectedPerm;
		playerlist_members.GetItemData(row, 0, selectedPerm);
		if (selectedPerm)
			return selectedPerm.param3;
		return -1;
	}
	
	override void InitMainWidget() {
		OBLLogger.Debug("InitMainWidget GrupManage Page");
		btn_kick = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_kick"));
		btn_leave = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_leave"));
		btn_promote = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_promote"));
		btn_demote = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_demote"));
		// гравці більше не обирають підгрупу вручну — кнопку ховаємо
		btn_joinSubgroup = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_joinSubgroup"));
		if (btn_joinSubgroup)
			btn_joinSubgroup.Show(false);
		
		searchbox = EditBoxWidget.Cast(rootWidget.FindAnyWidget("searchbox"));
		buttonInvite = ButtonWidget.Cast(rootWidget.FindAnyWidget("buttonInvite"));
		
		txt_groupname = TextWidget.Cast(rootWidget.FindAnyWidget("txt_groupname"));
		
		playerlist_online = TextListboxWidget.Cast(rootWidget.FindAnyWidget("playerlist_online"));
		playerlist_members = TextListboxWidget.Cast(rootWidget.FindAnyWidget("playerlist_members"));
	}
}
