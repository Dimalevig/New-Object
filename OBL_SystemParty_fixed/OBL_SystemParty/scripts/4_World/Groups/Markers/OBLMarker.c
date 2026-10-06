class OBLMarker {
	
	static ref array<OBLMarker> allMarkers = new array<OBLMarker>();
	static bool hideAllMarkers = false;

	OBLMarkerType type;
	int uid;
	string name;
	string icon;
	vector position = vector.Zero;
	int currentSubgroup;
	int colorA = 255, colorR = 255, colorG = 255, colorB = 255;
	
	[NonSerialized()]
	static bool streamerMode = OBLLayoutConfig.Get().streamerModeEnabled;
	[NonSerialized()]
	bool visibleOnScreen = false;
	[NonSerialized()]
	Widget mainWidget;
	[NonSerialized()]
	Widget bottomWidget;
	[NonSerialized()]
	ImageWidget iconWidget;
	[NonSerialized()]
	TextWidget nameWidget;
	[NonSerialized()]
	TextWidget distanceWidget;
	[NonSerialized()]
	OBLParty parentGroup;
	[NonSerialized()]
	ref MarkerConfigEntry cachedMarkerConfig = null;
	[NonSerialized()]
	float dist;
	[NonSerialized()]
	bool show = true;
	[NonSerialized()]
	bool disable3dDifferentSubgroup = false;
	[NonSerialized()]
	int state = 0;
	[NonSerialized()]
	int lastcolor = 0;
	[NonSerialized()]
	bool showBottom = true;
	[NonSerialized()]
	float lastFade = -1;
	[NonSerialized()]
	float iconBaseW = -1, iconBaseH = -1;
	
	// плавне згасання й зменшення далеких 3D-маркерів
	static const float FADE_NEAR = 150;
	static const float FADE_FAR = 1500;
	static const float FADE_MIN_ALPHA = 0.45;
	static const float FADE_MIN_SCALE = 0.7;
	
	[NonSerialized()]
	Widget compassWidget;
	[NonSerialized()]
	ImageWidget compassIconWidget;
	[NonSerialized()]
	TextWidget compassNameWidget;
	
	static bool compassInit = false;
	static float currentCameraAngle = 0;
	static Widget compassWidgetGlobal;
	
	static void InitCompassWidgets() {
		compassInit = true;
		foreach (OBLMarker marker : allMarkers) {
			if (marker) {
				marker.InitCompassWidget();
			}
		}
	}
	
	static void UpdateAllMarkers() {
		foreach (OBLMarker marker : allMarkers) {
			if (marker) {
				if (!hideAllMarkers && marker.UpdateMarkerClient())
					marker.SetWidgetPosition();
				else {
					marker.SetVisibleOnScreen(false);
					if (marker.compassWidget)
						marker.compassWidget.Show(false);
				}
			}
		}
	}
	
	static void UpdateAllMarkersSlow() {
		//OBLLogger.Debug("UpdateAllMarkersSlow " + allMarkers.Count());
		foreach (OBLMarker marker : allMarkers) {
			if (marker) {
				marker.UpdateMarkerSlow();
			}
		}
	}

	void InitSimpleClient(string namee, vector pos, string ic, int r, int g, int b) {
		type = OBLMarkerType.SERVER_STATIC;
		uid = Math.RandomInt(200, int.MAX - 1);
		name = namee;
		position = pos;
		icon = ic;
		colorA = 255;
		colorR = r;
		colorG = g;
		colorB = b;
	}

	
	void SetupMarker(OBLMarkerType type_, string name_, string icon_, vector pos_) {
		this.type = type_;
		this.name = name_;
		this.icon = icon_;
		this.position = pos_;
		this.uid = Math.RandomInt(200, int.MAX - 1);
	}

	void SetColorInt(int color_) {
		int a_ = (color_ >> 24) & 0xFF;
		int r_ = (color_ >> 16) & 0xFF;
		int g_ = (color_ >> 8) & 0xFF;
		int b_ = color_ & 0xFF;

		colorA = a_;
		colorR = r_;
		colorG = g_;
		colorB = b_;
	}
	
	void InitMarker() {
		if (GetMarkerConfig() && !GetMarkerConfig().display3d)
			return;
		if (mainWidget || GetGame().IsServer())
			return;
		mainWidget = GetGame().GetWorkspace().CreateWidgets("OBL_SystemParty/gui/layouts/3dmarker.layout", null);
		if (mainWidget) {
			iconWidget = ImageWidget.Cast(mainWidget.FindAnyWidget("icon"));
			nameWidget = TextWidget.Cast(mainWidget.FindAnyWidget("name"));
			distanceWidget = TextWidget.Cast(mainWidget.FindAnyWidget("distance"));
			bottomWidget = mainWidget.FindAnyWidget("bottom");
			
			if (GetIcon().Length() > 0) {
				OBLLogger.Debug("Loading Image: " + GetIcon());
				iconWidget.LoadImageFile(0, GetIcon());
			} else {
				iconWidget.Show(false);
			}
			if (type != OBLMarkerType.GROUP_PING) {
				nameWidget.SetText(name);
			} else {
				nameWidget.SetText("");
				nameWidget.Show(false);
			}
			mainWidget.Update();
			UpdateDistance();
			
			SetColor(true);
		}
		SetVisibleOnScreen(false);
		string printname = name + "";
		printname.Replace("%", "");
		OBLLogger.Debug("Init Marker " + printname + ". Created Layout: " + (mainWidget != null));
		UpdateMarkerSlow();
		if (OBLPartyMainConfig.Get().enableCompassHud)
			InitCompassWidget();
	}
	
	void InitCompassWidget() {
		if (GetMarkerConfig() && !GetMarkerConfig().displayCompass)
			return;
		if (compassWidget || GetGame().IsServer())
			return;
		if (!compassInit)
			return;
		compassWidget = GetGame().GetWorkspace().CreateWidgets(GetCompassMarkerLayout(), compassWidgetGlobal);
		if (compassWidget) {
			compassIconWidget = ImageWidget.Cast(compassWidget.FindAnyWidget("icon"));
			compassNameWidget = TextWidget.Cast(compassWidget.FindAnyWidget("name"));
			if (GetIcon().Length() > 0) {
				OBLLogger.Debug("Loading Image: " + GetIcon());
				compassIconWidget.LoadImageFile(0, GetIcon());
			} else {
				compassIconWidget.Show(false);
			}
			compassNameWidget.SetText(name);
			compassWidget.Show(false);
			SetColor(true);
		}
	
	}
	
	string GetIcon() {
		if (type == OBLMarkerType.GROUP_PING)
			return OBLMarkerVisibilityManager.Get().GetPingMarkerIcon();
		return icon;
	}
	
	string GetCompassMarkerLayout() {
		return OBLLayoutConfig.Get().GetCurrentLayout("Compass Marker");
	}
	
	void UpdateMarkerSlow() {
		SetColor();
		// підгрупи тепер лише статус «онлайн/офлайн», тож маркери за ними не ховаємо
		// (офлайн-гравці й так не мають 3D-маркера — це перевіряє ShowMarker)
		disable3dDifferentSubgroup = false;
		show = ShowMarker();
		//OBLLogger.Debug("Show Marker: " + name + ": " + show);
	}
	
	void OnMarkerRPCClient(int type_, ParamsReadContext ctx) {
		if (type_ == OBLPartyRPCs.POSITION) {
			float x,y,z;
			if (!ctx.Read(x) || !ctx.Read(y) || !ctx.Read(z))
				return;
			vector vec = Vector(x,y,z);
			SetPosition(vec);
		} else if (type_ == OBLPartyRPCs.SUBGRUOP) {
			int grp = 0;
			if (!ctx.Read(grp))
				return;
			SetSubGroup(grp);
		} else if (type_ == OBLPartyRPCs.NAME) {
			string name_;
			if (!ctx.Read(name_))
				return;
			SetName(name_);
		} else if (type_ == OBLPartyRPCs.COLOR) {
			int a,r,g,b;
			if (!ctx.Read(a) || !ctx.Read(r) || !ctx.Read(g) || !ctx.Read(b))
				return;
			SetColorARGB(a,r,g,b);
			SetColor(true);
		}
	}
	
	void AddToAllList() {
		allMarkers.Insert(this);
	}
	
	void RemoveFromAllList() {
		allMarkers.RemoveItem(this);
	}
	
	void OBLMarker() {
		AddToAllList();
	}
	
	void SetSubGroup(int grp) {
		currentSubgroup = grp;
	}
	
	void SetPosition(vector pos) {
		position = pos;
		if (type == OBLMarkerType.PRIVATE_MARKER) {
			OBLPrivateMarkerManager.Get().Save();
		}
	}
	
	void SendPositionToServer() {
		// OBL FIX: only group markers/pings live on the server; private markers are saved locally in SetPosition
		if (type != OBLMarkerType.GROUP_MARKER && type != OBLMarkerType.GROUP_PING)
			return;
		ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.POSITION);
		rpc.Write(position[0]);
		rpc.Write(position[1]);
		rpc.Write(position[2]);
		SendMarkerRPC(rpc);
	}
	
	void SetName(string name_) {
		if (this.name != name_) {
			this.name = name_;
		}
	}
	
	void ~OBLMarker() {
		OBLLogger.Debug("Removed Marker: " + name);
		if (allMarkers) {
			allMarkers.RemoveItem(this);
		}
		if (mainWidget) {
			mainWidget.Unlink();
			mainWidget = null;
		}
		if (compassWidget) {
			compassWidget.Unlink();
			compassWidget = null;
		}
	}
	
	void SetColor(bool force = false) {
		int rgb = Get3DColorARGB();
		if (force || lastcolor != rgb) {
			if (iconWidget)
				iconWidget.SetColor(rgb);
			if (nameWidget)
				nameWidget.SetColor(rgb);
			if (compassNameWidget && compassIconWidget) {
				compassNameWidget.SetColor(rgb);
				compassIconWidget.SetColor(rgb);
			}
			if (distanceWidget) {
				distanceWidget.SetColor(ARGB(colorA, 255, 255, 255));
			}
			lastcolor = rgb;
		}
	}

	bool SetColorARGB(int a, int r, int g, int b) {
		if (colorA != a || colorR != r || colorG != g || colorB != b) {
			colorA = a;
			colorR = r;
			colorG = g;
			colorB = b;
			return true;
		}
		return false;
	}

	bool SetIcon(string icon_) {
		if (this.icon != icon_) {
			this.icon = icon_;
			if (iconWidget)
				iconWidget.LoadImageFile(0, icon);
			return true;
		}
		return false;
	}
	
	bool SetColorARGBGlobal(int a, int r, int g, int b) {
		if (SetColorARGB(a,r,g,b)) {
			ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.COLOR);
			rpc.Write(a);
			rpc.Write(r);
			rpc.Write(g);
			rpc.Write(b);
			OBLLogger.Debug("Sending Color Change: " + a + " " + r + " " + g + " " + b);
			SendMarkerRPC(rpc);
			return true;
		}
		return false;
	}
	
	bool SetIconGlobal(string icon_) {
		if (SetIcon(icon_)) {
			ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.ICON);
			rpc.Write(icon_);
			SendMarkerRPC(rpc);
			return true;
		}
		return false;
	}

	bool SetNameT(string name_) {
		if (this.name != name_) {
			this.name = name_;
			if (nameWidget)
				nameWidget.SetText(name);
			return true;
		}
		return false;
	}
	
	bool SetNameGlobal(string name_) {
		if (SetNameT(name_)) {
			ScriptRPC rpc = CreateRPCCall(OBLPartyRPCs.NAME);
			rpc.Write(name_);
			SendMarkerRPC(rpc);
			return true;
		}
		return false;
	}
	
	MarkerConfigEntry GetMarkerConfig() {
		if (cachedMarkerConfig)
			return cachedMarkerConfig;
		cachedMarkerConfig = OBLPartyMainConfig.Get().GetMarkerConfigEntry(type);
		return cachedMarkerConfig;
	}
	
	bool ShowDistance() {
		if (!GetMarkerConfig())
			return true;
		return GetMarkerConfig().displayDistance;
	}
	
	bool ShouldCenterWidget() {
		return type == OBLMarkerType.GROUP_PING;
	}
	
	float GetCompassPosY() {
		if (type != OBLMarkerType.GROUP_PING)
			return 0;
		return 0.5;
	}
	
	void SetDistance() {
		if (!GetGame() || !GetGame().GetPlayer() || !ShowDistance() || !distanceWidget) {
			if (showBottom && bottomWidget)
				bottomWidget.Show(false, false);
			showBottom = false;
			return;
		} else if (!showBottom) {
			if (bottomWidget)
				bottomWidget.Show(true, false);
			showBottom = true;
		}
		vector pos = GetGame().GetCurrentCameraPosition();
		dist = vector.Distance(position, pos);
		if (dist < 1000) {
			distanceWidget.SetText(GetDistancePrefix() + ((int) dist) + "m");
		} else {
			float km = ((float) ((int) (dist / 100))) / 10;
			distanceWidget.SetText(GetDistancePrefix() + km + "km");
		}
	}
	
	// текст перед відстанню (напр. «НОКАУТ · »)
	string GetDistancePrefix() {
		return "";
	}
	
	bool UsesDistanceFade() {
		return type != OBLMarkerType.GROUP_PING;
	}
	
	void UpdateDistanceFade() {
		if (!mainWidget || !UsesDistanceFade())
			return;
		float t = (dist - FADE_NEAR) / (FADE_FAR - FADE_NEAR);
		t = Math.Clamp(t, 0, 1);
		// оновлюємо лише при помітній зміні, щоб не смикати віджети щокадру
		if (lastFade >= 0 && Math.AbsFloat(t - lastFade) < 0.02)
			return;
		lastFade = t;
		mainWidget.SetAlpha(1.0 - t * (1.0 - FADE_MIN_ALPHA));
		if (iconWidget) {
			if (iconBaseW < 0)
				iconWidget.GetSize(iconBaseW, iconBaseH);
			float scale = 1.0 - t * (1.0 - FADE_MIN_SCALE);
			iconWidget.SetSize(iconBaseW * scale, iconBaseH * scale);
		}
	}
	
	bool ShowMarker() {
		if (streamerMode && (type == OBLMarkerType.SERVER_STATIC || type == OBLMarkerType.SERVER_DYNAMIC))
			return false;
		MarkerConfigEntry cfg = GetMarkerConfig();
	//	OBLLogger.Debug("MarkerEntry: " + cfg);
		if (!cfg)
			return false;
		vector pos = GetGame().GetCurrentCameraPosition();
		dist = vector.Distance(position, pos);
	//	OBLLogger.Debug("Dist: " + dist + " from: " + pos + " To: " + position);
		if (GetMarkerConfig().maxDistance < 0 || GetMarkerConfig().maxDistance > dist) {
			return OBLMarkerVisibilityManager.Get().Is3DVisiblie(uid, type);
		}
		return false;
	}
	
	void UpdateDistance() {
		if (ShowDistance()) {
			SetDistance();
			if (bottomWidget)
				bottomWidget.Show(true);
		} else if (bottomWidget) {
			bottomWidget.Show(false);
		}
	}
	
	int GetColorARGB() {
		if (type == OBLMarkerType.GROUP_PING) {
			return OBLColorManager.Get().GetColor("Ping 3D Marker");
		}
		return ARGB(colorA, colorR, colorG, colorB);
	}
	
	int Get3DColorARGB() {
		return GetColorARGB();
	}
	
	bool UpdateMarkerClient() {
		if (!mainWidget || !show || !GetGame() || !GetGame().GetPlayer() || !GetGame().GetPlayer().IsAlive() || GetGame().GetPlayer().IsUnconscious())
			return false;
		UpdateDistance();
		UpdateDistanceFade();
		return true;
	}
	
	bool SetWidgetPosition() {
		if (!mainWidget || !position)
			return false;
		if (compassWidget) {
			float angle = GetMarkerAngle();
			float posX = (angle / 180.0) - 0.5;
			if (posX < -1)
				posX += 2;
			compassWidget.SetPos(posX, GetCompassPosY());
		}
		vector screenPos = GetGame().GetScreenPos(position);
		int screenWidth, screenHeight;
		GetScreenSize(screenWidth,screenHeight);
		if (screenPos[0] <= 0 || screenPos[0] >= screenWidth || screenPos[1] <= 0 || screenPos[1] >= screenHeight || screenPos[2] <= 0) {
			SetVisibleOnScreen(false);
			return false;
		}
		if (ShouldCenterWidget()) {
			float width, height;
			mainWidget.GetScreenSize(width, height);
			screenPos[0] = screenPos[0] - width / 2;
			screenPos[1] = screenPos[1] - height / 2;
		}
		SetVisibleOnScreen(true);
		mainWidget.SetPos(screenPos[0], screenPos[1]);
		return true;
	}
	
	float GetMarkerAngle() {
		vector camPos = GetGame().GetCurrentCameraPosition();
		vector dir = camPos - position;
		dir = dir.Normalized();
		vector angles = dir.VectorToAngles();
		float angle = angles[0] - currentCameraAngle + 360;
		while (angle > 180)
			angle -= 360;
		return angle;
	}
	
	void SetVisibleOnScreen(bool b) {
		visibleOnScreen = b;
		if (mainWidget)
			mainWidget.Show(b && !disable3dDifferentSubgroup && show);
		if (compassWidget) {
			compassWidget.Show(!disable3dDifferentSubgroup && show);
		}
	}
	
	bool ReadFromCtx(ParamsReadContext ctx) {
		if (!ctx.Read(type))
			return false;
		if (!ctx.Read(uid))
			return false;
		if (!ctx.Read(name))
			return false;
		if (!ctx.Read(icon))
			return false;
		if (!ctx.Read(position))
			return false;
		if (!ctx.Read(colorA))
			return false;
		if (!ctx.Read(colorR))
			return false;
		if (!ctx.Read(colorG))
			return false;
		if (!ctx.Read(colorB))
			return false;
		if (!ctx.Read(currentSubgroup))
			return false;
		return true;
	}
	
	void WriteToCtx(ParamsWriteContext ctx) {
		ctx.Write(type);
		ctx.Write(uid);
		ctx.Write(name);
		ctx.Write(icon);
		ctx.Write(position);
		ctx.Write(colorA);
		ctx.Write(colorR);
		ctx.Write(colorG);
		ctx.Write(colorB);
		ctx.Write(currentSubgroup);
	}
	
	ScriptRPC CreateRPCCall(int type_) {
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(type_);
		rpc.Write(uid);
		return rpc;
	}
	
	void SendMarkerRPC(ScriptRPC rpc) {
		if (!parentGroup && GetGame().IsClient() && GetGame().IsMultiplayer()) {
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb)
				parentGroup = pb.GetOBLParty();
		}
		if (parentGroup) {
			if (GetGame().IsServer()) {
				parentGroup.SendRPCToGroupMembers(rpc);
			} else {
				parentGroup.SendRPCToServer(rpc);
			}
		} else if (type == OBLMarkerType.SERVER_STATIC || type == OBLMarkerType.SERVER_DYNAMIC) {
			rpc.Send(null, OBLPartyRPCs.MARKER_RPC, true);
		} else {
			if (!parentGroup)
				OBLLogger.Debug("Failed to send Marker RPC for Marker: " + type + " No Parent Group ! ");
			else
				OBLLogger.Debug("Failed to send Marker RPC for Marker: " + type + " Parent Group: " + parentGroup.shortname);
		}
	}
	
}