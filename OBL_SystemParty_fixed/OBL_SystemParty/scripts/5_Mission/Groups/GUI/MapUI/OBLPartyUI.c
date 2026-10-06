class OBLPartyUI : UIScriptedMenu {
	
	const int TOP_BUTTON_COUNT = 6;
	const int TOP_BUTTON_ACTIVE_COUNT = 3;

	bool typing = false;
	bool initialized = false, initializedRest = false;
	
	MapWidget mapWidget;
	ref OBLMapMarkerManager mapMarkerManager;
	
	Widget leftPanel, fullPanel, topPanel;
	ref array<ref OBLPartyPage> pages = new array<ref OBLPartyPage>();
	OBLPartyPage currentPage;
	ref OBLAddMarkerPopup addPopup;
	Widget groupButton = null;
	CheckBoxWidget chckbx_dragMarkers;
	
	string initializedLayout = "";
	
	void OBLPartyUI() {
		OBLLayoutConfig.Event_OnLayoutChanged.Insert(OnLayoutChange);
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);
	}
	void ~OBLPartyUI() {
		OBLLayoutConfig.Event_OnLayoutChanged.Remove(OnLayoutChange);
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
		OBLPartyPage.topButtons.Clear();
		foreach (OBLPartyPage page : pages) {
			delete page;
		}
		pages.Clear();
	}
	
	void OnStreamerModeChange(bool enabled) {
		OBLLogger.Debug("OnStreamerModeChange OBLPartyUI: " + enabled);
		if (!layoutRoot)
			return;
		ImageWidget serverLogo = ImageWidget.Cast(layoutRoot.FindAnyWidget("logo"));
		if (!enabled && serverLogo && OBLPartyMainConfig.Get().serverLogoPath.Length() > 0) {
			serverLogo.LoadImageFile(0, OBLPartyMainConfig.Get().serverLogoPath);
			serverLogo.Show(true);
		} else if (serverLogo)
			serverLogo.Show(false);
	}
	
	void OnLayoutChange() {
		if (initializedLayout != GetMapLayout()) {
			OBLLogger.Debug("ReInitAll");
			ReInitAll();
		}
	}
	
	void ReInitAll() {
		OBLDataSerializer data = new OBLDataSerializer();
		StoreAllWidgetData(data);
		initialized = false;
		initializedRest = false;
		HideMenu();
		ShowMenu();
		RestoreAllWidgetData(data);
	}
	
	void ShowMenu() {
		GetGame().GetUIManager().ShowScriptedMenu(this, null);
		InitPageRest();
		OnStreamerModeChange(OBLLayoutConfig.Get().streamerModeEnabled);
	}
	
	void HideMenu() {
		GetGame().GetUIManager().HideScriptedMenu(this);
	}
	
	void StoreAllWidgetData(OBLDataSerializer data) {
		int current = GetCurrentPage();
		data.Write(new Param1<int>(current));
		foreach (OBLPartyPage page : pages) {
			page.StoreAllWidgetData(data);
		}
		addPopup.StoreAllWidgetData(data);
	}
	
	void RestoreAllWidgetData(OBLDataSerializer data) {
		Param1<int> currentParam = Param1<int>.Cast(data.Read());
		SetCurrentPage(currentParam.param1);
		foreach (OBLPartyPage page : pages) {
			page.RestoreAllWidgetData(data);
		}
		addPopup.RestoreAllWidgetData(data);
	}
	
	string GetMapLayout() {
		return OBLLayoutConfig.Get().GetCurrentLayout("Map");
	}
	
	override Widget Init() {
		OBLLogger.Debug("Init GroupUI");
		if (initialized)
			return layoutRoot;
		super.Init();
		initializedLayout = GetMapLayout();
		layoutRoot = GetGame().GetWorkspace().CreateWidgets(initializedLayout);
		OBLLogger.Debug("Created Root Layout ? " + (layoutRoot != null) + " Path: " + initializedLayout);
		if (!layoutRoot)
			return null;

		leftPanel = layoutRoot.FindAnyWidget("leftPanel");
		fullPanel = layoutRoot.FindAnyWidget("fullPanel");
		topPanel = layoutRoot.FindAnyWidget("topPanel");
		OBLLogger.Debug("Found Left Panel ? " + (leftPanel != null) + " Found Full Panel ? " + (fullPanel != null) + " Found Top Panel ? " + (topPanel != null));
		if (!leftPanel || !fullPanel || !topPanel)
			return null;
		
		mapWidget = MapWidget.Cast(layoutRoot.FindAnyWidget("Map"));
		chckbx_dragMarkers = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("chckbx_dragMarkers"));
		initialized = true;
		return layoutRoot;
	}
	
	void InitPageRest() {
		OBLLogger.Debug("initializedRest: " + initializedRest);
		if (initializedRest)
			return;
		mapMarkerManager = new OBLMapMarkerManager(mapWidget);
		SetServerLogo();
		InitPages();
		ChangePageTo(pages.Get(0));
		OBLLogger.Debug("Init Page Rest");
		initializedRest = true;
	}
	
	void SetServerLogo() {
		ImageWidget serverLogo = ImageWidget.Cast(layoutRoot.FindAnyWidget("logo"));
		if (serverLogo && OBLPartyMainConfig.Get().serverLogoPath.Length() > 0) {
			serverLogo.LoadImageFile(0, OBLPartyMainConfig.Get().serverLogoPath);
			serverLogo.Show(true);
		} else if (serverLogo)
			serverLogo.Show(false);
	}
	
	void OpenGroupPage() {
		if (groupButton)
			SetCurrentPage(groupButton);
	}
	
	void InitPages() {
		OBLLogger.Debug("InitPages GroupUI");
		OBLPartyPage.topButtons.Clear();
		foreach (OBLPartyPage page : pages) {
			delete page;
		}
		pages.Clear();
		
		addPopup = new OBLAddMarkerPopup();
		addPopup.Init(this);//*/
		
		OBLInfoPage page1 = new OBLInfoPage();
		pages.Insert(page1);
		page1.InitPage(this);
		
		OBLPartyCreatePage page2 = new OBLPartyCreatePage();
		pages.Insert(page2);
		page2.InitPage(this);
		
		OBLPartyManagePage page3 = new OBLPartyManagePage();
		pages.Insert(page3);
		page3.InitPage(this);
		
		OBLMarkerListPage page4 = new OBLMarkerListPage();
		pages.Insert(page4);
		page4.InitPage(this);
		
		OBLClientSettingsPage page5 = new OBLClientSettingsPage();
		pages.Insert(page5);
		page5.InitPage(this);
		
		if (OBLPartyMainConfig.Get().enableShop) {
			OBLShopPage pageShop = new OBLShopPage();
			pages.Insert(pageShop);
			pageShop.InitPage(this);
		}
		
		if (MissionGameplay.groupAdmin) {
			OBLAdminPage page6 = new OBLAdminPage();
			pages.Insert(page6);
			page6.InitPage(this);
		}
		CreateCustomPages();
	}
	
	OBLPartyPage GetPageByName(string name) {
		foreach (OBLPartyPage page : pages) {
			if (page.buttonname == name)
				return page;
		}
		return null;
	}
	
	void CreateCustomPages() {
		
	}
	
	int GetCurrentPage() {
		return pages.Find(currentPage);
	}
	
	bool SetCurrentPage(int index) {
		if (index < 0 || index >= pages.Count())
			return false;
		OBLPartyPage page = pages.Get(index);
		if (!page)
			return false;
		return SetCurrentPage(page.buttonWidget);
	}
	
	bool SetCurrentPage(Widget buttonClicked) {
		if (!buttonClicked)
			return false;
		foreach (OBLPartyPage page : pages) {
			if (page.OnTopButtonClicked(buttonClicked)) {
				ChangePageTo(page);
				return true;
			}
		}
		return false;
	}
	
	void ReloadCurrentPage() {
		if (currentPage) {
			Widget btn = currentPage.buttonWidget;
			SetCurrentPage(btn);
		}			
	}
	
	void ChangePageTo(OBLPartyPage page) {
		if (!page)
			return;
		if (currentPage)
			currentPage.OnHide();
		currentPage = page;
		page.OnShow();
	}
	
	void OnMarkerChanged() {
		if (currentPage)
			currentPage.OnMarkerChanged();
	}
	
	override bool OnMouseEnter(Widget w, int x, int y) {
		if (super.OnMouseEnter(w, x, y))
			return true;
		typing = (w && EditBoxWidget.Cast(w));
		return false;
	}
	
	override bool OnClick(Widget w, int x, int y, int button) {
		if (super.OnClick(w, x, y, button))
			return true;
		if (SetCurrentPage(w))
			return true;
		// клік по вкладці, яка зараз недоступна — показуємо причину замість мовчазного ігнору
		foreach (OBLPartyPage p : pages) {
			if (w == p.buttonWidget && !p.IsAvailable()) {
				string reason = p.GetUnavailableReason();
				if (reason != "")
					NotificationSystem.AddNotificationExtended(4, "Система груп", reason, "set:ccgui_enforce image:MapDestroyed");
				return true;
			}
		}
		if (currentPage && currentPage.OnClick(w))
			return true;
		if (addPopup.OnClick(w))
			return true;
		if (w == chckbx_dragMarkers && mapMarkerManager) {
			mapMarkerManager.SetDragable(chckbx_dragMarkers.IsChecked());
		}
		return false;
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished) {
		if (super.OnChange(w, x, y, finished))
			return true;
		if (currentPage && currentPage.OnChange(w))
			return true;
		if (addPopup.OnChange(w))
			return true;
		return false;
	}
	
	override bool OnItemSelected(Widget w, int x, int y, int row, int  column,	int  oldRow, int  oldColumn) {
		if (super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn))
			return true;
		if (currentPage && currentPage.OnItemSelected(w, row, column))
			return true;
		return false;
	}
	
	override bool OnDoubleClick(Widget w, int x, int y, int button) {
		if (super.OnDoubleClick(w, x, y, button))
			return true;
		if (w == mapWidget) {
			if (button == 0) {
				addPopup.ShowPopup(x, y, false, null);
			} else if (button == 1) {
				vector mousePos = Vector(x + 10,y + 10,0); //compensa deslocamento
				vector mapPos = mapWidget.ScreenToMap(mousePos);
				OBLMarker marker = addPopup.FindMarkerInRadius(mapPos);
				OBLLogger.Debug("Edit Marker");
				if (marker)
					addPopup.ShowPopup(x, y, true, marker);
			}
			return true;
		}
		if (currentPage && currentPage.OnDoubleClick(w))
			return true;
		return false;
	}
	
	
	void OnGroupChanged() {
		if (currentPage && !currentPage.IsAvailable()) {
			if (!SetCurrentPage(groupButton))
				ChangePageTo(pages.Get(0));
		} else {
			ReloadCurrentPage();
		}
		OBLLogger.Debug("UI On Group Changed");
		foreach (OBLPartyPage page : pages)
			page.OnGroupChanged();
		if (addPopup) {
			addPopup.OnGroupChanged();
		}
	}
	
	override void Update(float timeslice) {
		super.Update(timeslice);
		if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress()) {
			//initialized = false;
			HideMenu();
		}
		if (currentPage)
			currentPage.OnUpdateFrame();
		if (addPopup)
			addPopup.OnUpdateFrame();
		if (mapMarkerManager)
			mapMarkerManager.UpdateFrame();
	}
	
	override void OnShow() {
		OBLLogger.Debug("OnShow OBLPartyUI CheckNewMarkers" );
		if (!initialized) {
			HideMenu();
			return;
		}
		super.OnShow();
		OBLMarker.hideAllMarkers = true;
		PPEffects.SetBlurMenu(0.7);
		GetGame().GetMission().GetHud().ShowHudUI(false);
		GetGame().GetMission().GetHud().ShowQuickbarUI(false);
		MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
		if (mission)
			mission.PlayerControlDisable( INPUT_EXCLUDE_ALL );
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateAll, 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckNewMarkers, 100, true);
		if (currentPage)
			currentPage.OnShow();
		AddMapMarker();
		AddCustomMarkersOnMapOpen();
	}
	
	override void OnHide() {
		if (!initialized)
			return;
		super.OnHide();
		OBLMarker.hideAllMarkers = false;
		PPEffects.SetBlurMenu(0);
		GetGame().GetMission().GetHud().ShowHudUI(true);
		GetGame().GetMission().GetHud().ShowQuickbarUI(true);
		MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
		if (mission)
			mission.PlayerControlEnable(false);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateAll);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CheckNewMarkers);
		if (currentPage)
			currentPage.OnHide();
		if (mapMarkerManager)
			mapMarkerManager.ClearMarkers();
	}
	
	void UpdateAll() {
		if (currentPage)
			currentPage.OnUpdateSlow();
	}
	
	int lastGroupMarkerCount = 0, lastServerMarkerCount = 0, lastPrivateMarkerCount = 0, lastPlayerMarkerCount = 0, lastCustomMarkerCount = 0;
	
	void AddMapMarker() {
		if (!mapMarkerManager)
			return;
		mapMarkerManager.ClearMarkers();
		AddGroupMarkers();
		AddServerMarkers();
		AddPrivateMarkers();
		AddPlayerMarker();
		AddCustomMarkers();
		mapMarkerManager.CutAllCircles();
		if (mapMarkerManager) {
			mapMarkerManager.UpdateFrame(true);
			if (chckbx_dragMarkers)
				mapMarkerManager.SetDragable(chckbx_dragMarkers.IsChecked());
		}
	}
	
	void CheckNewMarkers() {
		if (NeedMarkerRefresh())
			AddMapMarker();
	}
	
	bool NeedMarkerRefresh() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		bool need = false;
		if (pb && pb.GetOBLParty()) {
			OBLParty grp = pb.GetOBLParty();
			int groupMarkers = 0;
			groupMarkers += grp.markers.Count();
			groupMarkers += grp.pings.Count();
			groupMarkers += grp.members.Count();
			if (groupMarkers != lastGroupMarkerCount) {
				lastGroupMarkerCount = groupMarkers;
				need = true;
			}
		} else if (lastGroupMarkerCount != 0) {
			// OBL FIX: after leaving a group its markers stayed on the open map
			lastGroupMarkerCount = 0;
			need = true;
		}
		OBLStaticMarkerManagerClient mgr = OBLStaticMarkerManagerClient.Get();
		int serverMarkers = mgr.staticMarkers.Count();
		if (serverMarkers != lastServerMarkerCount) {
			lastServerMarkerCount = serverMarkers;
			need = true;
		}
		OBLPrivateMarkerManager mgrc = OBLPrivateMarkerManager.Get();
		int clientMarkers = mgrc.privateMarkers.Count();
		if (clientMarkers != lastPrivateMarkerCount) {
			lastPrivateMarkerCount = clientMarkers;
			need = true;
		}
		OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
		if (cfg && cfg.canSeeOwnPlayerOnMap && lastPlayerMarkerCount != 1) {
			lastPlayerMarkerCount = 1;
			need = true;
		} else if (cfg && !cfg.canSeeOwnPlayerOnMap && lastPlayerMarkerCount != 0) {
			lastPlayerMarkerCount = 0;
			need = true;
		}
		return need;
	}
	
	void AddCustomMarkersOnMapOpen() {
		if (!mapWidget)
			return;
		mapWidget.ClearUserMarks();
		#ifdef PVEZ
		array<ref PVEZ_LawbreakerMarker> bxdmarkers = g_Game.pvez_LawbreakersMarkers.markers;
		array<ref PVEZ_Zone> zones = g_Game.pvez_Zones.activeZones;
		int color = ARGB(255,
			g_Game.pvez_Config.MAP.Zones_Border_Color.R,	// red
			g_Game.pvez_Config.MAP.Zones_Border_Color.G,	// green
			g_Game.pvez_Config.MAP.Zones_Border_Color.B);	// blue	
		foreach (PVEZ_Zone zone : zones) {
			vector pos = Vector(zone.X, 0, zone.Z);
			if (zone.ShowNameOnMap) {
				mapWidget.AddUserMark(zone.GetVectorPos(), zone.Name, color, "");
			}
		}
		PVEZ_MapMarkersDrawer.DrawLawbreakers(mapWidget);
		#endif
		UpdateKothMarker();
	}
	
	void AddCustomMarkers() {
		//mapMarkerManager.AddCircleNonScaling(Vector(3709.30, 0, 5993.22), 300.0, ARGB(255, 0, 255, 0));
		#ifdef PVEZ
		if (!mapMarkerManager)
			return;
		array<ref PVEZ_Zone> zones = g_Game.pvez_Zones.activeZones;
		int color = ARGB(255,
			g_Game.pvez_Config.MAP.Zones_Border_Color.R,	// red
			g_Game.pvez_Config.MAP.Zones_Border_Color.G,	// green
			g_Game.pvez_Config.MAP.Zones_Border_Color.B);	// blue	
		foreach (PVEZ_Zone zone : zones) {
			if (zone.ShowBorderOnMap) {
				mapMarkerManager.AddCircleNonScaling(zone.GetVectorPos(), zone.Radius, color, 5488);
			}
		}
		#endif
	}
	
	void UpdateKothMarker() {
		#ifdef THKOTH
		MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
		mission.AddKOTHMarker(mapWidget, mapMarkerManager);
		#endif
	}
	
	void AddPlayerMarker() {
		OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
		if (!GetGame().GetPlayer() || ! cfg || !cfg.canSeeOwnPlayerOnMap)
			return;
		mapMarkerManager.AddMarkerObject(GetGame().GetPlayer(), "Я", OBLColorManager.Get().GetColor("Own Player Map Marker"), cfg.ownPlayerIconPath);
	}
	
	void AddGroupMarkers() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		OBLParty grp = pb.GetOBLParty();
		if (!grp)
			return;
		foreach (OBLMarker marker : grp.markers) {
			mapMarkerManager.AddMarker(marker);
		}
		foreach (OBLMarker marker2 : grp.pings) {
			mapMarkerManager.AddMarker(marker2);
		}
		OBLPartyMainConfig cfg = OBLPartyMainConfig.Get();
		foreach (OBLPartyMember member : grp.members) {
			member.icon = cfg.otherPlayerIconsPath;
			mapMarkerManager.AddMarker(member);
		}
	}
	
	bool AddServerMarkers() {
		OBLStaticMarkerManagerClient mgr = OBLStaticMarkerManagerClient.Get();
		foreach (OBLServerMarker marker : mgr.staticMarkers) {
			mapMarkerManager.AddMarker(marker);
			if (marker.DrawCircle())
				mapMarkerManager.AddCircleNonScaling(marker.position, marker.radius, ARGB(marker.colorA, marker.colorR, marker.colorG, marker.colorB));
		}
		return true;
	}
	
	bool AddPrivateMarkers() {
		OBLPrivateMarkerManager mgr = OBLPrivateMarkerManager.Get();
		foreach (OBLMarker marker : mgr.privateMarkers) {
			mapMarkerManager.AddMarker(marker);
		}
		return true;
	}
	
	override bool OnDrag(Widget w, int x, int y) {
		if (mapMarkerManager)
			mapMarkerManager.OnDragStart(w);
		OBLLogger.Debug("OnDrag: " + w);
		return true;
	}
	override bool OnDragging(Widget w, int x, int y, Widget reciever) {
		OBLLogger.Debug("OnDragging: " + w);
		return true;
	}
	override bool OnDraggingOver(Widget w, int x, int y, Widget reciever) {
		OBLLogger.Debug("OnDraggingOver: " + w);
		return true;
	}
	override bool OnDrop(Widget w, int x, int y, Widget reciever) {
		OBLLogger.Debug("OnDrop: " + w + " at: " + x + "," + y);
		if (!mapWidget)
			return true;
		vector worldpos = mapWidget.ScreenToMap(Vector(x + 10,y + 10,0));
		worldpos[1] = GetGame().SurfaceY(worldpos[0], worldpos[2]);
		if (mapMarkerManager) {
			OBLMarker marker = mapMarkerManager.FindMarkerByMainWidget(w);
			if (marker) {
				marker.SetPosition(worldpos);
				marker.SendPositionToServer();
			}
			mapMarkerManager.OnDragStop(w);
			mapMarkerManager.UpdateFrame(true);
		}
		return true;
	}
}
