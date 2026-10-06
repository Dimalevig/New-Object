class OBLCompassHud {

	ref Widget mainWidget;
	ref Widget compassImageWidget;
	ref Widget compassLineWidget;
	ref Widget topMarkerWidget;
	string initializedLayout = "";
	
	void OBLCompassHud() {
		OBLColorManager.Event_OnColorChange.Insert(OnColorChanged);
		OBLLayoutConfig.Event_OnLayoutChanged.Insert(OnLayoutChange);
	}
	
	void ~OBLCompassHud() {
		OBLColorManager.Event_OnColorChange.Remove(OnColorChanged);
		OBLLayoutConfig.Event_OnLayoutChanged.Remove(OnLayoutChange);
		if (mainWidget) {
			mainWidget.Unlink();
			mainWidget = null;
		}
		OBLMarker.compassInit = false;
	}
	
	void OnLayoutChange() {
		if (initializedLayout != GetCompassWidget()) {
			if (mainWidget) {
				mainWidget.Unlink();
				mainWidget = null;
			}
			OBLMarker.compassInit = false;
			InitWidgets();
		}
	}
	
	void InitWidgets() {
		if (mainWidget || GetGame().IsServer())
			return;
		initializedLayout = GetCompassWidget();
		mainWidget = GetGame().GetWorkspace().CreateWidgets(initializedLayout, null);
		if (mainWidget) {
			compassImageWidget = mainWidget.FindAnyWidget("compassimage");
			topMarkerWidget = mainWidget.FindAnyWidget("topMarkerWidget"); 
			compassLineWidget = mainWidget.FindAnyWidget("compassLineWidget"); 
			OBLMarker.compassWidgetGlobal = topMarkerWidget;
		}
		OBLLogger.Debug("Initialized Compass Hud Widget");
		OBLMarker.InitCompassWidgets();
		OnColorChanged();
	}
	
	string GetCompassWidget() {
		return OBLLayoutConfig.Get().GetCurrentLayout("Compass");
	}
	
	void Show(bool b) {
		if (mainWidget)
			mainWidget.Show(b);
	}
	
	void UpdateHud() {
		IngameHud hud = IngameHud.Cast(GetGame().GetMission().GetHud());
		if (hud) {
			bool visible = hud.OBLIsHudVisible() && GetGame().GetPlayer() && OBLMarkerVisibilityManager.Get().compassEnabled && GetGame().GetPlayer().IsAlive() && !GetGame().GetPlayer().IsUnconscious();
			Show(visible);
			if (visible && compassImageWidget) {
				float angle = GetCurrentAngle();
				compassImageWidget.SetPos((-angle / 180.0) - 1.0, 0);
				OBLMarker.currentCameraAngle = angle;
			}
		} else {
			Show(false);
		}
	}
	
	void OnColorChanged() {
		if (compassImageWidget)
			compassImageWidget.SetColor(OBLColorManager.Get().GetColor("Compass"));
		if (compassLineWidget)
			compassLineWidget.SetColor(OBLColorManager.Get().GetColor("Compass Line"));
	}
	
	float GetCurrentAngle() {
		vector dir = GetGame().GetCurrentCameraDirection();
		vector angles = dir.VectorToAngles();
		if (angles[0] > 180)
			return angles[0] - 360;
		return angles[0];
	}
	
}
