class OBLMarkerListEntry {

	OBLPartyUI groupUI;
	ref OBLMarker marker;
	
	Widget mainWidget;
	TextWidget name;
	ButtonWidget btn_0, btn_1, btn_2;
	ImageWidget icon;
	
	string spacername;
	bool spacer = false;
	bool expanded = true;
	OBLMarkerType type;
	
	void ~OBLMarkerListEntry(){
		if (mainWidget)
			mainWidget.Unlink();
	}
	
	void InitSpacer(Widget parent, string namee, OBLMarkerType type_) {
		this.type = type_;
		spacer = true;
		spacername = namee;
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Creating Spacer with layout: " + GetMarkerListLayout());
		mainWidget = GetGame().GetWorkspace().CreateWidgets(GetMarkerListLayout(), parent);
		btn_0 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_0"));
		btn_1 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_1"));
		btn_2 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_2"));
		name = TextWidget.Cast(mainWidget.FindAnyWidget("name"));
		icon = ImageWidget.Cast(mainWidget.FindAnyWidget("icon"));
		Widget spacerWidget = mainWidget.FindAnyWidget("spacer");
		if (spacerWidget)
			spacerWidget.Show(false);
		icon.Show(false);
		btn_0.SetText("3D");
		btn_1.SetText("v");
		btn_2.SetText(" ");
		name.SetText(" " + namee);
		name.SetBold(true);
		mainWidget.SetSize(0, 40);
		Show(false);
	}
	
	void InitMarker(Widget parent, OBLMarker mmarker, OBLPartyUI groupui) {
		this.groupUI = groupui;
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Creating Marker with layout: " + GetMarkerListLayout());
		mainWidget = GetGame().GetWorkspace().CreateWidgets(GetMarkerListLayout(), parent);
		btn_0 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_0"));
		btn_1 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_1"));
		btn_2 = ButtonWidget.Cast(mainWidget.FindAnyWidget("btn_2"));
		name = TextWidget.Cast(mainWidget.FindAnyWidget("name"));
		icon = ImageWidget.Cast(mainWidget.FindAnyWidget("icon"));
		btn_0.SetText("3D");
		btn_1.SetText("E");
		btn_2.SetText("X");
		name.SetText(" " + mmarker.name);
		if (mmarker.icon.Length() > 0) {
			icon.LoadImageFile(0, mmarker.icon);
		} else {
			icon.Show(false);
		}
		icon.SetColor(mmarker.GetColorARGB());
		Show(false);
		InitMarker(mmarker);
	}
	
	string GetMarkerListLayout() {
		return OBLLayoutConfig.Get().GetCurrentLayout("Map Marker List Entry");
	}
	
	void InitMarker(OBLMarker mmarker) {
		marker = mmarker;
	}
	
	void UpdateWidget() {
		int state = 0;
		if (spacer) {
			state = OBLMarkerVisibilityManager.Get().GetGlobalVisibilityOrAdd(type).displaystate;
		} else {
			if (marker) {
				state = OBLMarkerVisibilityManager.Get().GetVisibilityOrAdd(marker.uid).displaystate;
			}
		}
		if (state == 0)
			btn_0.SetText("3D");
		else if (state == 1)
			btn_0.SetText("2D");
		else
			btn_0.SetText("O");
		Show(true);
	}
	
	bool OnButtonPressed(Widget w) {
		if (w == btn_0) {
			if (spacer) {
				OBLMarkerVisibilityManager.Get().GetGlobalVisibilityOrAdd(type).GetNextState();
			} else if (marker) {
				OBLMarkerVisibilityManager.Get().GetVisibilityOrAdd(marker.uid).GetNextState();
			}
			return true;
		} else if (w == btn_1) {
			if (spacer) {
				if (expanded) {
					Collapse();
				} else {
					Expand();
				}
			} else {
				// OBL FIX: editing a player marker / foreign server marker created a duplicate group marker
				if (groupUI && CanEditMarker())
					groupUI.addPopup.ShowPopup(0, 0, true, marker);
			}
			return true;
		} else if (w == btn_2) {
			if (spacer) {
				if (expanded) {
					Collapse();
				} else {
					Expand();
				}
			} else {
				if (groupUI && CanEditMarker())
					groupUI.addPopup.RequestMarkerDelete(marker);
			}
			return true;
		}
		return false;
	}

	
	bool CanEditMarker() {
		if (!marker)
			return false;
		if (marker.type == OBLMarkerType.GROUP_MARKER || marker.type == OBLMarkerType.PRIVATE_MARKER)
			return true;
		if (marker.type == OBLMarkerType.SERVER_STATIC || marker.type == OBLMarkerType.SERVER_DYNAMIC)
			return MissionGameplay.groupAdmin;
		return false;
	}
	
	void Show(bool b) {
		if (mainWidget)
			mainWidget.Show(b);
	}
	
	void Expand() {
		expanded = true;
		if (btn_1)
			btn_1.SetText("v");
	}
	
	void Collapse() {
		expanded = false;
		if (btn_1)
			btn_1.SetText("^");
	}

}