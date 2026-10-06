class OBLMarkerList {

	Widget listWidget;
	
	ref array<ref OBLMarkerListEntry> entries = new array<ref OBLMarkerListEntry>();
	
	void CreateEntries(OBLPartyUI groupUI) {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb)
			return;
		OBLParty grp = pb.GetOBLParty();
		if (!OBLLayoutConfig.Get().streamerModeEnabled) {
			AddSpacer("Серверні маркери", OBLMarkerType.SERVER_STATIC);
			foreach (OBLServerMarker serverMarker : OBLStaticMarkerManagerClient.Get().staticMarkers) {
				AddEntry(serverMarker, groupUI);
			}
		}
		if (grp) {
			OBLPartyPermission perm = pb.GetPermission();
			if (perm && perm.CanSeeMarkerType(OBLMarkerType.GROUP_MARKER)) {
				AddSpacer("Маркери групи", OBLMarkerType.GROUP_MARKER);
				foreach (OBLMarker groupMarker : grp.markers) {
					AddEntry(groupMarker, groupUI);
				}
			}
			if (perm && perm.CanSeeMarkerType(OBLMarkerType.GROUP_PLAYER_MARKER)) {
				AddSpacer("Гравці", OBLMarkerType.GROUP_PLAYER_MARKER);
				foreach (OBLPartyMember member : grp.members) {
					AddEntry(member, groupUI);
				}
			}
		}
		AddSpacer("Приватні маркери", OBLMarkerType.PRIVATE_MARKER);
		foreach (OBLMarker privateMarker : OBLPrivateMarkerManager.Get().privateMarkers) {
			AddEntry(privateMarker, groupUI);
		}
	}
	
	void AddSpacer(string name, OBLMarkerType type) {
		OBLMarkerListEntry entry = new OBLMarkerListEntry();
		entry.InitSpacer(listWidget, name, type);
		entries.Insert(entry);
	}
	
	void AddEntry(OBLMarker marker, OBLPartyUI groupUI) {
		OBLMarkerListEntry entry = new OBLMarkerListEntry();
		entry.InitMarker(listWidget, marker, groupUI);
		entries.Insert(entry);
	}
	
	int ShouldUpdateList() {
		if (entries.Count() != 0)
			return 2;
		return 1;
	}
	
	void ClearAndDeleterEntries() {
		foreach (OBLMarkerListEntry entry : entries) {
			delete entry;
		}
		entries.Clear();
	}
	
	void UpdateEntries(OBLPartyUI groupUI, bool force = false) {
		if (!listWidget)
			return;
		int update = ShouldUpdateList();
		if (update == 1 || force) {
			ClearAndDeleterEntries();
			CreateEntries(groupUI);
		}
		OBLLogger.Debug("Updating MarkerList");
		bool expanded = true;
		foreach (OBLMarkerListEntry entry : entries) {
			if (entry.spacer) {
				expanded = entry.expanded;
				entry.Show(true);
				entry.UpdateWidget();
				continue;
			}
			if (expanded) {
				entry.Show(true);
				entry.UpdateWidget();
			} else {
				entry.Show(false);
			}
		}
	}
	
	bool OnButtonPressed(Widget w) {
		foreach (OBLMarkerListEntry entry : entries) {
			if (entry.OnButtonPressed(w))
				return true;
		}
		return false;
	}
	
}