class OBLPartyPage {
	
	static ref map<int, ButtonWidget> topButtons = new map<int, ButtonWidget>();
	
	ref OBLPartyUI parent;
	string buttonname;
	bool fullsized = false;
	
	ButtonWidget buttonWidget;
	ref Widget rootWidget;
	
	void ~OBLPartyPage() {
		if (rootWidget) {
			rootWidget.Unlink();
		}
		if (buttonWidget) {
			buttonWidget.Unlink();
		}
	}
	
	bool InitPage(OBLPartyUI parentUI) {
		return false;
	}
	
	void StoreAllWidgetData(OBLDataSerializer data) {
		topButtons.Clear();
	}
	
	void RestoreAllWidgetData(OBLDataSerializer data) {
		
	}
	
	bool InitPage(OBLPartyUI parentUI, int pageID, int pageSubID, string name, bool fullsize) {
		parent = parentUI;
		buttonname = name;
		fullsized = fullsize;
		return InitWidgets(pageID, pageSubID);
	}
	
	bool InitWidgets(int pageID, int pageSubID) {
		string layoutPath = OBLLayoutConfig.Get().GetCurrentPageLayout(pageID, pageSubID);
		Widget parentWidget;
		if (fullsized) {
			parentWidget = parent.fullPanel;
		} else {
			parentWidget = parent.leftPanel;
		}
		rootWidget = GetGame().GetWorkspace().CreateWidgets(layoutPath, parentWidget);
		rootWidget.Show(false);
		// сторінка створюється схованою — відразу прибираємо з hit-test,
		// щоб до першого OnShow не перехоплювала кліки інших сторінок
		rootWidget.SetFlags(WidgetFlags.IGNOREPOINTER);
		InitMainWidget();
		
		if (!topButtons.Contains(pageID)) {
			layoutPath = OBLLayoutConfig.Get().GetCurrentLayout("Map Top Button");
			if (OBLLogger.IsDebug())
				OBLLogger.Debug("Creating new Top Button for PageID: " + pageID + " with Layout " + layoutPath);
			Widget btnWid = GetGame().GetWorkspace().CreateWidgets(layoutPath, parent.topPanel);
			buttonWidget = ButtonWidget.Cast(btnWid);
			if (buttonWidget) {
				buttonWidget.SetText(buttonname);
				topButtons.Insert(pageID, buttonWidget);
				float width, height, posX, posY;
				buttonWidget.GetSize(width, height);
				buttonWidget.GetPos(posX, posY);
				// місце за порядком вкладок, а не за номером сторінки (інакше лишались «дірки»)
				posX += (width + 0.005) * (topButtons.Count() - 1);
				buttonWidget.SetPos(posX, posY);
			}
		} else {
			if (OBLLogger.IsDebug())
				OBLLogger.Debug("Getting Top Button for PageID: " + pageID + " with Layout " + layoutPath);
			buttonWidget = topButtons.Get(pageID);
		}
		return true;
	}
	
	bool OnTopButtonClicked(Widget w) {
		return (w == buttonWidget && IsAvailable());
	}
	
	bool IsAvailable() {
		return true;
	}

	// причина недоступності вкладки (для підказки гравцю). "" = без підказки
	string GetUnavailableReason() {
		return "";
	}
	
	bool OnClick(Widget w) {
		return false;
	}
	
	bool OnChange(Widget w) {
		return false;
	}
	
	bool OnItemSelected(Widget w, int row, int column) {
		return false;
	}
	
	bool OnDoubleClick(Widget w) {
		return false;
	}
	
	void OnGroupChanged() {
		
	}
	
	void InitMainWidget() {
		
	}
	
	void OnShow() {
		if (rootWidget) {
			rootWidget.Show(true);
			rootWidget.ClearFlags(WidgetFlags.IGNOREPOINTER);
		}
		SetTabActive(true);
		parent.leftPanel.Show(!fullsized);
		parent.mapWidget.Show(!fullsized);
		parent.fullPanel.Show(fullsized);
	}
	
	// підсвітка активної вкладки кольором Oblivion
	void SetTabActive(bool active) {
		if (!buttonWidget)
			return;
		Widget border = buttonWidget.FindAnyWidget("border");
		if (active) {
			buttonWidget.SetTextColor(OBLTheme.AccentLight());
			if (border)
				border.SetColor(OBLTheme.Accent());
		} else {
			buttonWidget.SetTextColor(OBLTheme.Text());
			if (border)
				border.SetColor(ARGB(115, 123, 92, 255));
		}
	}
	
	void OnHide() {
		SetTabActive(false);
		if (rootWidget) {
			rootWidget.Show(false);
			// leftPanel має clipchildren 0 — приховані сторінки, що лежать z-вище
			// (Альянс/Магазин), перехоплюють кліки по кнопках активної сторінки
			// (Покинути/Кік/Підвищити у вкладці "Група"). Прибираємо її з hit-test.
			rootWidget.SetFlags(WidgetFlags.IGNOREPOINTER);
		}
	}
	
	void OnUpdateFrame() {
	}
	
	void OnUpdateSlow() {
	}
	
	void OnMarkerChanged() {
	}
}
