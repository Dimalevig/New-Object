class OBLMapMarkerManager {
	private MapWidget map_widget;
	private ref CanvasWidget drawCanvas;
	private Widget iconPane;
	
	private ref array<ref MapMarkerWrapper> markers = new array<ref MapMarkerWrapper>();
	vector lastMapPos = vector.Zero;
	float lastMapScale = 0.0;
	
	void OBLMapMarkerManager(MapWidget mapWidget) {
		if (!mapWidget)
			return;
		map_widget = mapWidget;
		iconPane = map_widget.FindAnyWidget("iconPane");
		drawCanvas = CanvasWidget.Cast(map_widget.FindAnyWidget("drawCanvas"));
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateSlowVisibility, 100, true);
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);
	}
	
	void ~OBLMapMarkerManager() {
		ClearMarkers();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateSlowVisibility);
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
	}
	
	void SetDragable(bool drag) {
		if (!map_widget)
			return;
		float scale = map_widget.GetScale();
		vector pos = map_widget.GetMapPos();
		foreach (MapMarkerWrapper wrap : markers) {
			MapMarkerWrapperOBLMarker mark;
			if (Class.CastTo(mark, wrap)) {
				mark.SetDragable(drag);
				mark.Update(scale, pos, map_widget);
				mark.UpdateVisibility();
			}
		}
	}
	
	void UpdateFrame(bool force = false) {
		if (!map_widget || !drawCanvas)
			return;
		float scale = map_widget.GetScale();
		vector pos = map_widget.GetMapPos();
		if (force || scale != lastMapScale || vector.Distance(pos, lastMapPos) > 0.01) {
			drawCanvas.Clear();
			foreach (MapMarkerWrapper wrapper : markers) {
				if (wrapper.IsVisible())
					wrapper.Update(scale, pos, map_widget);
			}
		} else {
			foreach (MapMarkerWrapper wrapper2 : markers) {
				if (wrapper2.IsVisible() && wrapper2.UpdateOptional())
					wrapper2.Update(scale, pos, map_widget);
			}
		}
		lastMapScale = scale;
		lastMapPos = pos;
	}
	
	void OnStreamerModeChange(bool enabled) {
		UpdateSlowVisibility();
	}
	
	void UpdateSlowVisibility() {
		//OBLLogger.Debug("UpdateSlowVisibility " + markers.Count());
		foreach (MapMarkerWrapper wrapper : markers) {
			wrapper.UpdateVisibility();
		}
	}
	
	void ClearMarkers() {
		markers.Clear();
	}
	
	private void AddMarker(MapMarkerWrapper wrapper) {
		markers.Insert(wrapper);
		if (map_widget && wrapper.IsVisible()) {
			float scale = map_widget.GetScale();
			vector pos = map_widget.GetMapPos();
			wrapper.Update(scale, pos, map_widget);
			wrapper.UpdateVisibility();
			UpdateFrame();
		}
	}
	
	MapMarkerWrapper AddMarker(OBLMarker marker, int layer = 0) {
		MapMarkerWrapperOBLMarker wrapper = new MapMarkerWrapperOBLMarker();
		wrapper.Init(marker, iconPane);
		wrapper.layer = layer;
		AddMarker(wrapper);
		return wrapper;
	}
	
	MapMarkerWrapper AddMarkerObject(Object obj, string name = "", int color = 0xFFFFFFFF, string icon = "", int layer = 0) {
		MapMarkerWrapperObject wrapper = new MapMarkerWrapperObject();
		wrapper.Init(obj, color, icon, name, iconPane);
		wrapper.layer = layer;
		AddMarker(wrapper);
		return wrapper;
	}
	
	MapMarkerWrapper AddCircleNonScaling(vector pos, float radius, int color, int layer = 0) {
		MapMarkerWrapperCircle wrapper = new MapMarkerWrapperCircle();
		wrapper.Init(pos, radius, color, drawCanvas);
		wrapper.layer = layer;
		AddMarker(wrapper);
		return wrapper;
	}
	
	void RemoveLayer(int layer) {
		for (int i = 0; i < markers.Count(); i++) {
			MapMarkerWrapper wrapper = markers.Get(i);
			if (wrapper && wrapper.layer == layer) {
				markers.Remove(i);
				i--;
			}
		}
		UpdateFrame(true);
	}
	
	void CutAllCircles() {
		map<int, ref array<MapMarkerWrapperCircle>> circles = new map<int, ref array<MapMarkerWrapperCircle>>();
		for (int i = 0; i < markers.Count(); i++) {
			MapMarkerWrapper wrapper = markers.Get(i);
			MapMarkerWrapperCircle circle;
			if (Class.CastTo(circle, wrapper)) {
				array<MapMarkerWrapperCircle> arr;
				if (circles.Contains(wrapper.layer))
					arr = circles.Get(wrapper.layer);
				else {
					arr = new array<MapMarkerWrapperCircle>();
					circles.Insert(wrapper.layer, arr);
				}
				arr.Insert(circle);
			}
		}
		OBLLogger.Debug("Circle Layers: " + circles.Count());
		foreach (int layer, array<MapMarkerWrapperCircle> circ : circles) {
			OBLLogger.Debug("Layer: " + layer + " Circles: " + circ.Count());
			foreach (MapMarkerWrapperCircle cir : circ) {
				cir.SetOtherCircles(circ);
			}
		}
		UpdateFrame(true);
	}
	
	void RemoveMarker(OBLMarker marker) {
		for (int i = 0; i < markers.Count(); i++) {
			MapMarkerWrapper wrapper = markers.Get(i);
			MapMarkerWrapperOBLMarker wrapCast;
			if (!Class.CastTo(wrapCast, wrapper))
				continue;
			if (wrapCast && wrapCast.marker == marker) {
				markers.Remove(i);
				i--;
			}
		}
	}
	
	MapMarkerWrapper FindByMainWidget(Widget w) {
		for (int i = 0; i < markers.Count(); i++) {
			MapMarkerWrapper wrapper = markers.Get(i);
			if (wrapper.widget == w)
				return wrapper;
		}
		return null;
	}
	
	OBLMarker FindMarkerByMainWidget(Widget w) {
		MapMarkerWrapperOBLMarker wrap = MapMarkerWrapperOBLMarker.Cast(FindByMainWidget(w));
		if (!wrap)
			return null;
		return wrap.marker;
	}
	
	void OnDragStart(Widget w) {
		MapMarkerWrapper wrap = FindByMainWidget(w);
		if (!wrap)
			return;
		wrap.isDragged = true;
	}
	
	void OnDragStop(Widget w) {
		MapMarkerWrapper wrap = FindByMainWidget(w);
		if (!wrap)
			return;
		wrap.isDragged = false;
	}
	
}
class MapMarkerWrapper {

	float widgetWidth, widgetHeight;
	Widget widget;
	bool isDragged = false;
	
	int layer = 0;
	
	vector position = vector.Zero;
	vector lastPosition = vector.Zero;
	string lastname = "";
	string lasticon = "";
	int lastColor = 0;
	
	void ~MapMarkerWrapper() {
		if (widget) {
			widget.Unlink();
		}
	}
	
	void SetLayer(int layer_) {
		layer = layer_;
	}
	
	void Update(float mapScale, vector mapPos, MapWidget mapWidget) {
	}
	bool UpdateOptional() {
		return false;
	}
	void UpdateVisibility() {
	}
	
	bool IsVisible() {
		if (!widget)
			return false;
		return widget.IsVisible();
	}
	
	bool SetPosition(vector pos) {
		if (vector.Distance(lastPosition, pos) > 1) {
			this.position = pos;
			this.lastPosition = pos; 
			return true;
		}
		return false;
	}
	
	bool SetColor(int color) {
		if (lastColor != color) {
			lastColor = color;
			return true;
		}
		return false;
	}
}
class MapMarkerWrapperCircle : MapMarkerWrapper {

	const int CIRCLE_WIDTH = 2;
	
	float radius = 100;
	ref CanvasWidget drawCanvas;
	ref array<MapMarkerWrapperCircle> otherCircles;
	bool needPointUpdate = true;
	ref array<ref Param2<float, float>> intersectingAngles = new array<ref Param2<float, float>>();
	
	void Init(vector center, float radius_, int color, CanvasWidget drawCanvas_) {
		this.position = center;
		this.radius = radius_;
		this.lastColor = color;
		this.drawCanvas = drawCanvas_;
		drawCanvas.GetScreenSize(widgetWidth, widgetHeight);
	}
	
	override bool IsVisible() {
		return drawCanvas && drawCanvas.IsVisible();
	}
	
	override void Update(float mapScale, vector mapPos, MapWidget mapWidget) {
		int screenWidth, screenHeight;
		GetScreenSize(screenWidth, screenHeight);
		
		vector screenPos = mapWidget.MapToScreen(position);
		
		float radiusRoot = Math.Sqrt(radius);
		float circumference = 2 * Math.PI * radiusRoot + 1;
		float part = 1.0 / radiusRoot;
		float screenScale = screenHeight / 11500.0;
		float mapToScreen = mapScale / screenScale;
		
		float angle2 = 0;
		float rawX2 = radius, rawY2 = 0, newY2 = screenPos[1], newX2 = screenPos[0] + radius / mapToScreen;
		if (needPointUpdate) {
			CalcAllCircleIntersections();
		}
		bool hasIntersections = intersectingAngles.Count() > 0;
		int currentIntersectionIndex = 0;
		
		//OBLLogger.Debug("Drawing Parts: " + circumference + " " + needPointUpdate + " " + radius + " " + intersectingAngles.Count());
		for (int i = 1; i < circumference + 1; i++) {
			float angle1 = angle2;
			angle2 = part * i;
			float rawX1 = rawX2;
			rawX2 = radius * Math.Cos(angle2);
			float rawY1 = rawY2;
			rawY2 = -radius * Math.Sin(angle2);
			float newX1 = newX2;
			newX2 = screenPos[0] + rawX2 / mapToScreen;
			float newY1 = newY2;
			newY2 = screenPos[1] + rawY2 / mapToScreen;
			
			float avAng = (angle1 + angle2) / 2;
			
			if (!needPointUpdate) {
				if (!hasIntersections || currentIntersectionIndex >= intersectingAngles.Count()) {
					drawCanvas.DrawLine(newX1, newY1 , newX2 , newY2 , CIRCLE_WIDTH, lastColor);
					hasIntersections = false;
					continue;
				}
				Param2<float, float> inter = intersectingAngles.Get(currentIntersectionIndex);
				bool intersecting = false;
				bool edge = false;
				float angleEdge = 0;
				bool useNew2 = false;
				foreach (Param2<float, float> intersects : intersectingAngles) {
					if (angle2 > intersects.param1 && angle1 < intersects.param2) {
						if (intersects.param1 < angle1) {
							angleEdge = intersects.param2;
							useNew2 = true;
						} else {
							angleEdge = intersects.param1;
							useNew2 = false;
						}
						edge = true;
					}
					if (angle1 > intersects.param1 && angle2 < intersects.param2) {
						intersecting = true;
						break;
					}
				}
				if (intersecting) {
					continue;
				}
				if (edge) {
					if (useNew2) {
						float newX3 = screenPos[0] + radius * Math.Cos(angleEdge + 0.0015) / mapToScreen;
						float newY3 = screenPos[1] - radius * Math.Sin(angleEdge + 0.0015) / mapToScreen;
						drawCanvas.DrawLine(newX3, newY3 , newX2 , newY2 , CIRCLE_WIDTH, lastColor);
					} else {
						newX3 = screenPos[0] + radius * Math.Cos(angleEdge - 0.0015) / mapToScreen;
						newY3 = screenPos[1] - radius * Math.Sin(angleEdge - 0.0015) / mapToScreen;
						drawCanvas.DrawLine(newX3, newY3 , newX1 , newY1 , CIRCLE_WIDTH, lastColor);
					}
					continue;
				}
				drawCanvas.DrawLine(newX1, newY1 , newX2 , newY2 , CIRCLE_WIDTH, lastColor);
			}
		}
	}
	
	void CalcAllCircleIntersections() {
		intersectingAngles.Clear();
		if (!otherCircles)
			return;
		foreach (MapMarkerWrapperCircle circle : otherCircles) {
			if (circle == this)
				continue;
			float d = Math.Sqrt((position[0] - circle.position[0]) * (position[0] - circle.position[0]) + (position[2] - circle.position[2]) * (position[2] - circle.position[2]));
			if (d >= radius + circle.radius)
				continue;
			float a = (radius * radius - circle.radius * circle.radius + d * d) / 2 / d;
			float h = Math.Sqrt(radius * radius - a * a);
			float hd = h / d;
			float x3 = position[0] + (circle.position[0] - position[0]) * a / d;
			float y3 = position[2] + (circle.position[2] - position[2]) * a / d;
			
			float x1 = (x3 + hd * (circle.position[2] - position[2]) - position[0]);
			float y1 = (y3 - hd * (circle.position[0] - position[0]) - position[2]);
			
			float x2 = (x3 - hd * (circle.position[2] - position[2]) - position[0]);
			float y2 = (y3 + hd * (circle.position[0] - position[0]) - position[2]);
			
			float dist1 = Math.Sqrt(y1 * y1 + x1 * x1);
			float dist2 = Math.Sqrt(y2 * y2 + x2 * x2);
			float angle1 = Math.Acos(x1 / dist1);
			if (y1 < 0)
				angle1 = Math.PI2 - angle1;
			float angle2 = Math.Acos(x2 / dist2);
			if (y2 < 0)
				angle2 = Math.PI2 - angle2;
			/*
			float diff = Math.AbsFloat(angle1 - angle2);
			float angle12 = angle1 + diff / 2;
			if (angle12 > Math.PI2)
				angle12 -= Math.PI2;
			float angle21 = angle1 - diff / 2;
			if (angle21 > Math.PI2)
				angle21 -= Math.PI2;
			
			float x1_1 = radius * Math.Cos(angle12);
			float y1_1 = -radius * Math.Sin(angle12);
			
			float x2_1 = radius * Math.Cos(angle21);
			float y2_1 = -radius * Math.Sin(angle21);
			
			float d_1 = Math.Sqrt((x1_1 - circle.position[0]) * (x1_1 - circle.position[0]) + (y1_1 - circle.position[2]) * (y1_1 - circle.position[2]));
			float d_2 = Math.Sqrt((x2_1 - circle.position[0]) * (x2_1 - circle.position[0]) + (y2_1 - circle.position[2]) * (y2_1 - circle.position[2]));
			
			if (d_2 < d_1) {
				OBLLogger.Debug("Changing Angles " + angle1 + " and " + angle2);
				float temp = angle1;
				angle1 = angle2;
				angle2 = temp;
			}*/
			
			OBLLogger.Debug("Angles for " + position + ": " + angle1 + " " + angle2 + " " + y1 + " " + x1 + "   " + y2 + " " + x2);
			
			if (angle1 < angle2) {
				intersectingAngles.Insert(new Param2<float, float>(angle1, angle2));
			} else {
				intersectingAngles.Insert(new Param2<float, float>(-1, angle2));
				intersectingAngles.Insert(new Param2<float, float>(angle1, Math.PI2 + 1));
			}
		}
		needPointUpdate = false;
		//RearrangeIntersections();
	}
	
	void RearrangeIntersections() {
		array<ref Param2<float, float>> recalculated = new array<ref Param2<float, float>>();
		float minStart = -1;
		for (int i = 0; i < intersectingAngles.Count(); i++) {
			float end = minStart;
			float start = -1;
			foreach (Param2<float, float> inter : intersectingAngles) {
				if ((inter.param1 < start || start == -1) && inter.param1 > minStart) {
					start = inter.param1;
					end = inter.param2;
				}
			}
			if (start == -1)
				break;
			bool changed = true;
			while (changed) {
				changed = false;
				foreach (Param2<float, float> inter2 : intersectingAngles) {
					if (inter2.param1 < end && inter2.param1 > start && inter2.param2 > end) {
						end = inter2.param2;
						changed = true;
					}
				}
			}
			if (start == end)
				break;
			Param2<float, float> insert = new Param2<float, float>(start, end);
			recalculated.Insert(insert);
			minStart = end;
		}
		OBLLogger.Debug("Rearranged Angles from:");
		foreach (Param2<float, float> parm : intersectingAngles) {
			OBLLogger.Debug("    " + parm.param1 + " " + parm.param2);
		}
		OBLLogger.Debug("to:");
		foreach (Param2<float, float> rec : recalculated) {
			OBLLogger.Debug("    " + rec.param1 + " " + rec.param2);
		}
		intersectingAngles = recalculated;
	}
	
	void SetOtherCircles(array<MapMarkerWrapperCircle> circles) {
		otherCircles = circles;
		needPointUpdate = true;
	}
}
class MapMarkerWrapperObject : MapMarkerWrapper {

	Object obj;
	
	ImageWidget icon;
	TextWidget name;
	
	void Init(Object obj2, int color, string theicon, string thename, Widget iconPane) {
		if (!iconPane)
			return;
		obj = obj2;
		widget = GetGame().GetWorkspace().CreateWidgets("OBL_SystemParty/gui/layouts/mapmenu/map_marker.layout", iconPane);
		if (widget) {
			widget.Show(true);
			name = TextWidget.Cast(widget.FindAnyWidget("name"));
			icon = ImageWidget.Cast(widget.FindAnyWidget("icon"));
			SetIcon(theicon);
			SetColor(color);
			SetName("  " + thename);
			SetPosition(obj.GetPosition());
			widget.Update();
			widget.GetScreenSize(widgetWidth, widgetHeight);
		}
	}
	
	override void Update(float mapScale, vector mapPos, MapWidget mapWidget) {
		if (isDragged || !widget)
			return;
		vector screenPos = mapWidget.MapToScreen(GetPosition());
		
		widget.SetPos(screenPos[0] - widgetHeight / 2, screenPos[1] - widgetHeight / 2, true);
		if (name)
			name.SetTextExactSize(Math.Clamp(20.0, 10, 50));
		if (icon) {
			icon.SetSize(widgetHeight, widgetHeight);
		}
	}
	
	override void UpdateVisibility() {
		if (!widget)
			return;
		bool visible = IsMarkerVisible();
		widget.Show(visible);
	}
	
	bool IsMarkerVisible() {
		if (obj && PlayerBase.Cast(obj)) {
			OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
			return cfg && cfg.canSeeOwnPlayerOnMap;
		}
		return false;
	}
	
	vector GetPosition() {
		if (obj)
			return obj.GetPosition();
		return position;
	}
	
	
	override bool UpdateOptional() {
		if (SetPosition(GetPosition())) {
			return true;
		}
		return false;
	}
	
	bool SetIcon(string iconpath) {
		if (lasticon != iconpath && icon) {
			icon.LoadImageFile(0, iconpath);
			lasticon = iconpath;
			return true;
		}
		return false;
	}
	
	bool SetName(string thename) {
		if (lastname != thename) {
			name.SetText(thename);
			lastname = thename;
			return true;
		}
		return false;
	}
}
class MapMarkerWrapperOBLMarker : MapMarkerWrapper {

	OBLMarker marker;
	ImageWidget icon;
	TextWidget name;
	bool dragable = false;
	Widget iconPane_;
	
	void Init(OBLMarker marker2, Widget iconPane) {
		iconPane_ = iconPane;
		if (!iconPane)
			return;
		marker = marker2;
		OBLLogger.Debug("Marker Layout: " + GetLayout());	
		widget = GetGame().GetWorkspace().CreateWidgets(GetLayout(), iconPane);
		if (widget) {
			widget.Show(true);
			name = TextWidget.Cast(widget.FindAnyWidget("name"));
			icon = ImageWidget.Cast(widget.FindAnyWidget("icon"));
			SetIcon(marker.icon);
			SetColor(marker.GetColorARGB());
			SetName(" " + marker.name);
			SetPosition(marker.position);
			widget.Update();
			widget.GetScreenSize(widgetWidth, widgetHeight);
		}
	}
	
	void ReInit() {
		if (widget)
			widget.Unlink();
		lastPosition = vector.Zero;
		lastname = "";
		lasticon = "";
		lastColor = 0;
		Init(marker, iconPane_);
	}
	
	string GetLayout() {
		if (dragable && marker && (marker.type == OBLMarkerType.GROUP_MARKER || marker.type == OBLMarkerType.PRIVATE_MARKER))
			return "OBL_SystemParty/gui/layouts/mapmenu/map_marker_dragable.layout";
		return "OBL_SystemParty/gui/layouts/mapmenu/map_marker.layout";
	}
	
	void SetDragable(bool drag_) {
		if (drag_ != dragable) {
			dragable = drag_;
			ReInit();
		}
	}
	
	override void Update(float mapScale, vector mapPos, MapWidget mapWidget) {
		if (isDragged || !widget)
			return;
		vector screenPos = mapWidget.MapToScreen(GetPosition());
		
		widget.SetPos(screenPos[0] - widgetHeight / 2, screenPos[1] - widgetHeight / 2, true);
		if (name)
			name.SetTextExactSize(Math.Clamp(20.0, 10, 50));
		if (icon) {
			icon.SetSize(widgetHeight, widgetHeight);
		}
	}
	
	override void UpdateVisibility() {
		if (!widget)
			return;
		if (marker && (marker.type == OBLMarkerType.SERVER_STATIC || marker.type == OBLMarkerType.SERVER_DYNAMIC) && OBLLayoutConfig.Get().streamerModeEnabled) {
			widget.Show(false);
			return;
		}
		bool visible = IsMarkerVisible();
		widget.Show(visible);
	}
	
	bool IsMarkerVisible() {
		if (!marker)
			return false;
		if (!marker.GetMarkerConfig().displayMap)
			return false;
		if (!OBLMarkerVisibilityManager.Get().IsMapVisible(marker.uid, marker.type, false))
			return false;
		OBLMarkerType type = marker.type;
		if (type != OBLMarkerType.GROUP_PING && !OBLMarkerVisibilityManager.Get().IsGlobal2DVisible(type) || type == OBLMarkerType.GROUP_PING && !OBLMarkerVisibilityManager.Get().IsGlobal2DVisible(OBLMarkerType.GROUP_MARKER))
			return false;
		OBLServerMarker serverMarker;
		if (Class.CastTo(serverMarker, marker)) {
			return serverMarker.displayMap;
		}
		if (type == OBLMarkerType.GROUP_PING || type == OBLMarkerType.GROUP_MARKER || type == OBLMarkerType.GROUP_PLAYER_MARKER) {
			
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pb || !pb.GetOBLParty())
				return false;
			OBLParty grp = pb.GetOBLParty();
			OBLPartyPermission myPerm = pb.GetPermission();
			if (!myPerm)
				return false;
			
			if (!myPerm.CanSeeMarkerType(type))
				return false;
			OBLPartyMember myMarker = pb.GetMyGroupMarker();
			if (!myMarker)
				return false;
			OBLPartyMember memberMarker;
			if (Class.CastTo(memberMarker, marker)) {
				string mysteamid = MissionGameplay.Cast(GetGame().GetMission()).mySteamid;
				// підгрупи = статус онлайн/офлайн: тіммейтів показуємо всіх, незалежно від підгрупи
				return memberMarker.steamid != mysteamid;
			}
		}
		return true;
	}
	
	vector GetPosition() {
		if (marker)
			return marker.position;
		return position;
	}
	
	override bool SetColor(int color) {
		if (lastColor != color) {
			name.SetColor(color);
			icon.SetColor(color);
		}
		return super.SetColor(color);
	}
	override bool UpdateOptional() {
		if (marker) {
			if (SetPosition(marker.position) | SetColor(marker.GetColorARGB()) | SetIcon(marker.icon) | SetName(marker.name)) {
				return true;
			}
		}
		return false;
	}
	
	bool SetIcon(string iconpath) {
		if (lasticon != iconpath && icon) {
			icon.LoadImageFile(0, iconpath);
			lasticon = iconpath;
			return true;
		}
		return false;
	}
	
	bool SetName(string thename) {
		if (lastname != thename) {
			name.SetText(thename);
			lastname = thename;
			return true;
		}
		return false;
	}
}
