class OBLMarkerListPage : OBLPartyPage {
	
	ref OBLMarkerList markerListManger;
		
	void OBLMarkerListPage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);
	}
	
	void ~OBLMarkerListPage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
	}
	
	void OnStreamerModeChange(bool enabled) {
		if (markerListManger)
			markerListManger.UpdateEntries(parent, true);
	}

	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 2, 0, "Маркери", false);
	}
	
	int lastMarkerCount = 0;
	
	override void InitMainWidget() {
		markerListManger = new OBLMarkerList();
		GridSpacerWidget marker_list = GridSpacerWidget.Cast(rootWidget.FindAnyWidget("marker_list"));
		markerListManger.listWidget = marker_list;
	}
	
	override void OnUpdateSlow() {
		UpdateMarkerListManager();
	}
	
	override bool OnClick(Widget w) {
		if (markerListManger && markerListManger.OnButtonPressed(w)) {
			markerListManger.UpdateEntries(parent);
			return true;
		}
		return false;
	}
	
	override void OnMarkerChanged() {
		if (markerListManger)
			markerListManger.UpdateEntries(parent, true);
	}
	
	override void OnShow() {
		super.OnShow();
		UpdateMarkerListManager();
		OnStreamerModeChange(OBLLayoutConfig.Get().streamerModeEnabled);
	}
	
	void UpdateMarkerListManager() {
		int count = GetMarkerCount();
		OBLLogger.Debug("GroupUI: UpdateMarkerListManager " + (markerListManger != null) + " " + count);
		if (markerListManger)
			markerListManger.UpdateEntries(parent, count != lastMarkerCount);
		lastMarkerCount = count;
	}
	
	int GetMarkerCount() {
		int count = 0;
		count += OBLPrivateMarkerManager.Get().privateMarkers.Count();
		count += OBLStaticMarkerManagerClient.Get().staticMarkers.Count();
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (pb && pb.GetOBLParty()) {
			count += pb.GetOBLParty().members.Count();
			count += pb.GetOBLParty().markers.Count();
		}
		return count;
	}
	
}
