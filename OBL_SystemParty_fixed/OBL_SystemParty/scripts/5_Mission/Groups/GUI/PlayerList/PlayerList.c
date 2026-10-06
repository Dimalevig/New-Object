class OBLPlayerList {

	ref Widget mainWidget;
	ref Widget listWidget;
	string initializedLayout = "";
	ref Timer m_UpdateTimer;
	
	ref array<ref OBLPlayerListEntry> entries = new array<ref OBLPlayerListEntry>();
	
	static ref OBLPlayerList g_OBLPlayerList;
	
	static void Delete() {
		if (g_OBLPlayerList)
			delete g_OBLPlayerList;
	}
	
	static OBLPlayerList Get() {
		if (!g_OBLPlayerList) {
			g_OBLPlayerList = new OBLPlayerList();
			g_OBLPlayerList.InitWidgets();
		}
		return g_OBLPlayerList;
	}
	
	void OBLPlayerList() {
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateList, 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateDistances, 200, true);
		OBLColorManager.Event_OnColorChange.Insert(OnColorChange);
		OBLPositionManager.Event_OnPositionChange.Insert(OnPositionChange);
		OBLLayoutConfig.Event_OnLayoutChanged.Insert(OnLayoutChange);
		StartUpdateListTimer();
	}
	
	void ~OBLPlayerList() {
		StopUpdateListTimer();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateList);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateDistances);
		OBLColorManager.Event_OnColorChange.Remove(OnColorChange);
		OBLPositionManager.Event_OnPositionChange.Remove(OnPositionChange);
		OBLLayoutConfig.Event_OnLayoutChanged.Remove(OnLayoutChange);
		foreach (OBLPlayerListEntry entry : entries) {
			delete entry;
		}
		entries.Clear();
		if (mainWidget)
			mainWidget.Unlink();
	}
	
	void OnLayoutChange() {
		UpdateEntries(true);
	}
	
	void OnColorChange() {
		UpdateEntries(true);
	}
	
	void OnPositionChange() {
		UpdatePosition();
	}
	
	void UpdatePosition() {
		if (!listWidget)
			return;
		OBLLogger.Debug("Updating Position of Playerlist");
		vector pos = OBLPositionManager.Get().GetPosition("PlayerList");
		int index = OBLPositionManager.Get().GetIndex("PlayerList");
		OBLWidgetUtils.SetWidgetAlignmentIndex(listWidget, index);
		OBLWidgetUtils.SetWidgetPositionIndex(listWidget, pos, index);
		OBLLogger.Debug("Updated Position to: " + pos);
	}
	
	void UpdateVisibility() {
		if (!GetGame() || !GetGame().GetMission() || !mainWidget)
			return;
		IngameHud hud = IngameHud.Cast(GetGame().GetMission().GetHud());
		if (!hud) {
			mainWidget.Show(false);
			return;
		}
		mainWidget.Show(hud.OBLIsHudVisible() && OBLMarkerVisibilityManager.Get().playerlistEnabled);
	}
	
	void InitWidgets() {
		if (mainWidget || GetGame().IsServer())
			return;
		initializedLayout = GetPlayerListWidget();
		mainWidget = GetGame().GetWorkspace().CreateWidgets(initializedLayout, null);
		if (mainWidget) {
			listWidget = mainWidget.FindAnyWidget("playerlist");
		}
		OBLLogger.Debug("Initialized PlayerList Widget");
		OnGroupChanged();
	}
	
	string GetPlayerListWidget() {
		return "OBL_SystemParty/gui/layouts/playerlist/playerlist.layout";
	}
	
	void OnGroupChanged() {
		OnColorChange();
		OnPositionChange();
		OnLayoutChange();
	}
	
	void CreateEntries() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		OBLParty grp = pb.GetOBLParty();
		OBLPartyMember myMarker = pb.GetMyGroupMarker();
		if (!myMarker)
			return;
		int mySubGroup = myMarker.currentSubgroup;
		array<ref OBLPartyMember> members = grp.GetSubgroupMembers(mySubGroup);
		foreach (OBLPartyMember member : members) {
			OBLPlayerListEntry entry = new OBLPlayerListEntry();
			entry.Init(this, member);
			entries.Insert(entry);
		}
	}
	
	int ShouldUpdateList() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty()) {
			if (entries.Count() > 0)
				return 1;
			return 0;
		}
		OBLParty grp = pb.GetOBLParty();
		OBLPartyMember myMarker = pb.GetMyGroupMarker();
		if (!myMarker) {
			if (entries.Count() > 0)
				return 1;
			return 0;
		}
		int mySubGroup = myMarker.currentSubgroup;
		array<ref OBLPartyMember> members = grp.GetSubgroupMembers(mySubGroup);
		OBLLogger.Debug("Group Members: " + members.Count());
		if (members.Count() != entries.Count())
			return 1;
		bool changedHealth = false;
		foreach (OBLPlayerListEntry entry : entries) {
			if (!entry || !entry.member || members.Find(entry.member) == -1)
				return 1;
			if (entry.lastHealth != entry.member.health || entry.lastDowned != entry.member.downed)
				changedHealth = true;
		}
		if (changedHealth)
			return 2;
		return 0;
	}
	
	void ClearAndDeleterEntries() {
		foreach (OBLPlayerListEntry entry : entries) {
			delete entry;
		}
		entries.Clear();
	}

	void UpdateList() {
		UpdateEntries(false);
	}

	void StartUpdateListTimer()
	{
		if (!m_UpdateTimer)
			m_UpdateTimer = new Timer();

		// Para evitar múltiplas chamadas, primeiro cancela se já estiver rodando
		m_UpdateTimer.Stop();

		// Chama a função a cada 60 segundos
		m_UpdateTimer.Run(60.0, this, "UpdateList", NULL, true);
	}

	void StopUpdateListTimer()
	{
		if (m_UpdateTimer)
			m_UpdateTimer.Stop();
	}

	void UpdateEntries(bool force = false) {
		OBLLogger.Debug("UpdateEntries. Force ? " + force + " Count: " + entries.Count());
		if (!mainWidget)
			return;
		int update = ShouldUpdateList();
		OBLLogger.Debug("Update: " + update);
		if (update == 0 && !force)
			return;
		if (update == 1 || force) {
			ClearAndDeleterEntries();
			CreateEntries();
		}
		//OBLLogger.Debug("Updating PlayerList Widgets. Entry Count: " + entries.Count());
		foreach (OBLPlayerListEntry entry : entries) {
			entry.UpdateWidget();
		}
		//OBLLogger.Debug("Updated all PlayerList Widgets");
	}
	
	void UpdateDistances() {
		foreach (OBLPlayerListEntry entry : entries) {
			entry.UpdateDistance();
		}
	}
	
}
