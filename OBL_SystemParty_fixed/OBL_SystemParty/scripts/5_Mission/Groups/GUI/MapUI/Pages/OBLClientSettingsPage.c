class OBLClientSettingsPage : OBLPartyPage {
	
	const int CHAT_SIZE_START = 7;
	
	SliderWidget sliderA, sliderR, sliderG, sliderB;
	EditBoxWidget editA, editR, editG, editB;
	Widget colorPreview;
	TextListboxWidget availableColors;
	ButtonWidget toDefault;
	
	ButtonWidget btn_ResetAll, btn_Reload_From_Config, btn_Save_Config;
	XComboBoxWidget comboboxPingIcon, comboboxChatSize, comboboxTeammate3DDistance;
	ImageWidget imagePingIcon;
	CheckBoxWidget chckbx_Playerlist, chckbx_Compass, chckbx_ShowChatTag, chckbx_ShowClantextures, chckbx_StreamerMode;
	
	SliderWidget sliderY, sliderX;
	Widget previewIcon;
	TextListboxWidget availableLayoutsList;
	CheckBoxWidget invertX, invertY;
	ButtonWidget toDefaultPosition;
	
	TextListboxWidget availableLayoutsListStyles;
	XComboBoxWidget comboBoxStyle, comboPlayerlistStyleTemp;
	ButtonWidget toDefaultStyle;
	
	ref TStringArray colorArray = new TStringArray();
	ref TStringArray positionsArray = new TStringArray();
	ref TStringArray stylesArray = new TStringArray();
	ref TStringArray pingIconsArray = new TStringArray();

	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 3, 0, "Налаштування", true);
	}
	
	override void StoreAllWidgetData(OBLDataSerializer data) {
		data.Write(new Param3<int, int, int>(availableColors.GetSelectedRow(), availableLayoutsList.GetSelectedRow(), availableLayoutsListStyles.GetSelectedRow()));
	}
	
	override void RestoreAllWidgetData(OBLDataSerializer data) {
		Param3<int, int, int> selectedParam = Param3<int, int, int>.Cast(data.Read());
		availableColors.SelectRow(selectedParam.param1);
		availableColors.EnsureVisible(selectedParam.param1);
		OnItemSelected(availableColors, selectedParam.param1, 0);
		availableLayoutsList.SelectRow(selectedParam.param2);
		availableLayoutsList.EnsureVisible(selectedParam.param2);
		OnItemSelected(availableLayoutsList, selectedParam.param2, 0);
		availableLayoutsListStyles.SelectRow(selectedParam.param3);
		availableLayoutsListStyles.EnsureVisible(selectedParam.param3);
		OnItemSelected(availableLayoutsListStyles, selectedParam.param3, 0);
	}
	
	override void InitMainWidget() {
		sliderA = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderA"));
		sliderR = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderR"));
		sliderG = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderG"));
		sliderB = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderB"));
		editA = EditBoxWidget.Cast(rootWidget.FindAnyWidget("editA"));
		editR = EditBoxWidget.Cast(rootWidget.FindAnyWidget("editR"));
		editG = EditBoxWidget.Cast(rootWidget.FindAnyWidget("editG"));
		editB = EditBoxWidget.Cast(rootWidget.FindAnyWidget("editB"));
		toDefault = ButtonWidget.Cast(rootWidget.FindAnyWidget("toDefault"));
		colorPreview = rootWidget.FindAnyWidget("colorPreview");
		availableColors = TextListboxWidget.Cast(rootWidget.FindAnyWidget("availableColors"));
		
		btn_ResetAll = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_ResetAll"));
		btn_Reload_From_Config = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_Reload_From_Config"));
		btn_Save_Config = ButtonWidget.Cast(rootWidget.FindAnyWidget("btn_Save_Config"));
		comboboxPingIcon = XComboBoxWidget.Cast(rootWidget.FindAnyWidget("comboboxPingIcon"));
		comboboxChatSize = XComboBoxWidget.Cast(rootWidget.FindAnyWidget("comboboxChatSize"));
		comboboxTeammate3DDistance = XComboBoxWidget.Cast(rootWidget.FindAnyWidget("comboboxTeammate3DDistance"));
		imagePingIcon = ImageWidget.Cast(rootWidget.FindAnyWidget("imagePingIcon"));
		chckbx_Playerlist = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_Playerlist"));
		chckbx_Playerlist.Show(OBLPartyMainConfig.Get().enablePlayerList);
		chckbx_Compass = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_Compass"));
		chckbx_Compass.Show(OBLPartyMainConfig.Get().enableCompassHud);
		chckbx_ShowChatTag = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_ShowChatTag"));
		chckbx_ShowClantextures = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_ShowClantextures"));
		chckbx_StreamerMode = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("chckbx_StreamerMode"));
		bool showChat = false;
		#ifndef OBL_DISABLE_CHAT
		showChat = true;
		#endif
		comboboxChatSize.Show(showChat);
		chckbx_ShowChatTag.Show(showChat);
		rootWidget.FindAnyWidget("txtChatSize").Show(showChat);
		
		sliderY = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderY"));
		sliderX = SliderWidget.Cast(rootWidget.FindAnyWidget("sliderX"));
		previewIcon = rootWidget.FindAnyWidget("previewIcon");
		availableLayoutsList = TextListboxWidget.Cast(rootWidget.FindAnyWidget("availableLayoutsList"));
		invertX = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("checkInvertX"));
		invertY = CheckBoxWidget.Cast(rootWidget.FindAnyWidget("checkInvertY"));
		toDefaultPosition = ButtonWidget.Cast(rootWidget.FindAnyWidget("toDefaultPosition"));
		
		availableLayoutsListStyles = TextListboxWidget.Cast(rootWidget.FindAnyWidget("availableLayoutsListStyles"));
		comboBoxStyle = XComboBoxWidget.Cast(rootWidget.FindAnyWidget("comboBoxStyle"));
		comboPlayerlistStyleTemp = XComboBoxWidget.Cast(rootWidget.FindAnyWidget("comboPlayerlistStyleTemp"));
		toDefaultStyle = ButtonWidget.Cast(rootWidget.FindAnyWidget("toDefaultStyle"));
		
		comboPlayerlistStyleTemp.AddItem("За замовчуванням");
		comboPlayerlistStyleTemp.AddItem("Малий");
		comboPlayerlistStyleTemp.AddItem("Міні");
	}
	
	override void OnShow() {
		super.OnShow();
		ReloadAll();
	}
	
	void ReloadAll(bool force = false) {
		ReloadColors(force);
		ReloadPositions(force);
		ReloadPingMarkers(force);
		ReloadStyles();
		ReloadTextSizes();
		ReloadTeammate3DDistance();
		OnIconPreviewPositionChange();
		chckbx_Playerlist.SetChecked(OBLMarkerVisibilityManager.Get().playerlistEnabled);
		chckbx_Compass.SetChecked(OBLMarkerVisibilityManager.Get().compassEnabled);
		chckbx_ShowClantextures.SetChecked(!OBLMarkerVisibilityManager.Get().disableShowClantextures);
		chckbx_StreamerMode.SetChecked(OBLLayoutConfig.Get().streamerModeEnabled);
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		OBLPartyPermission permission;
		if (pb)
			permission = pb.GetPermission();
		chckbx_ShowChatTag.Enable(permission && permission.nextGroupUID == -1);
		chckbx_ShowChatTag.SetChecked(pb && pb.GetOBLParty() != null && pb.GetOBLParty().showTagInChat);
	}
	
	void ReloadStyles() {
		if (comboPlayerlistStyleTemp && comboPlayerlistStyleTemp.GetNumItems() > 0)
			comboPlayerlistStyleTemp.SetCurrentItem(OBLLayoutConfig.Get().playerlistLayoutIndex % comboPlayerlistStyleTemp.GetNumItems());
	}
	
	void ReloadColors(bool force = false) {
		TStringArray colorArray2 = OBLColorManager.Get().GetColorStrings();
		if (force || colorArray2.Count() != colorArray.Count()) {
			availableColors.ClearItems();
			colorArray = colorArray2;
			foreach (string s : colorArray)
				availableColors.AddItem(GetColorDisplayName(s), null, 0);
			availableColors.SelectRow(0);
			availableColors.EnsureVisible(0);
		}
	}
	
	void ReloadPositions(bool force = false) {
		TStringArray positionsArray2 = OBLPositionManager.Get().GetPositionStrings();
		if (force || positionsArray2.Count() != positionsArray.Count()) {
			availableLayoutsList.ClearItems();
			positionsArray = positionsArray2;
			foreach (string s : positionsArray)
				availableLayoutsList.AddItem(GetPositionDisplayName(s), null, 0);
			availableLayoutsList.SelectRow(0);
			availableLayoutsList.EnsureVisible(0);
		}
	}
	
	string GetColorDisplayName(string colorName) {
		switch (colorName) {
			case "Player 3D Marker":
				return "3D-маркер гравця";
			case "Own Player Map Marker":
				return "Маркер свого гравця на мапі";
			case "Player Online":
				return "Гравець онлайн";
			case "Player Offline":
				return "Гравець офлайн";
			case "Ping 3D Marker":
				return "3D-маркер пінгу";
			case "Compass":
				return "Компас";
			case "Compass Line":
				return "Лінія компаса";
			case "Playerlist entry full health":
				return "Список гравців: повне здоров'я";
			case "Playerlist entry zero health":
				return "Список гравців: нуль здоров'я";
			case "Playerlist entry border":
				return "Список гравців: рамка";
		}
		return colorName;
	}
	
	string GetPositionDisplayName(string positionName) {
		switch (positionName) {
			case "PlayerList":
				return "Список гравців";
			case "Chat":
				return "Чат";
		}
		return positionName;
	}
	
	void ReloadTeammate3DDistance() {
		if (!comboboxTeammate3DDistance)
			return;
		comboboxTeammate3DDistance.ClearAll();
		comboboxTeammate3DDistance.AddItem("MAX (сервер)");
		comboboxTeammate3DDistance.AddItem("OFF");
		comboboxTeammate3DDistance.AddItem("250 м");
		comboboxTeammate3DDistance.AddItem("500 м");
		comboboxTeammate3DDistance.AddItem("1 км");
		comboboxTeammate3DDistance.AddItem("1.5 км");
		comboboxTeammate3DDistance.AddItem("2 км");
		comboboxTeammate3DDistance.AddItem("3 км");
		comboboxTeammate3DDistance.AddItem("5 км");

		int current = OBLMarkerVisibilityManager.Get().GetTeammate3DMarkerDistance();
		switch (current) {
			case 0:
				comboboxTeammate3DDistance.SetCurrentItem(1);
				break;
			case 250:
				comboboxTeammate3DDistance.SetCurrentItem(2);
				break;
			case 500:
				comboboxTeammate3DDistance.SetCurrentItem(3);
				break;
			case 1000:
				comboboxTeammate3DDistance.SetCurrentItem(4);
				break;
			case 1500:
				comboboxTeammate3DDistance.SetCurrentItem(5);
				break;
			case 2000:
				comboboxTeammate3DDistance.SetCurrentItem(6);
				break;
			case 3000:
				comboboxTeammate3DDistance.SetCurrentItem(7);
				break;
			case 5000:
				comboboxTeammate3DDistance.SetCurrentItem(8);
				break;
			default:
				comboboxTeammate3DDistance.SetCurrentItem(0);
				break;
		}
	}

	void OnTeammate3DDistanceChanged() {
		if (!comboboxTeammate3DDistance)
			return;
		int selected = comboboxTeammate3DDistance.GetCurrentItem();
		int distance = -1;
		switch (selected) {
			case 1:
				distance = 0;
				break;
			case 2:
				distance = 250;
				break;
			case 3:
				distance = 500;
				break;
			case 4:
				distance = 1000;
				break;
			case 5:
				distance = 1500;
				break;
			case 6:
				distance = 2000;
				break;
			case 7:
				distance = 3000;
				break;
			case 8:
				distance = 5000;
				break;
		}
		OBLMarkerVisibilityManager.Get().SetTeammate3DMarkerDistance(distance);
	}

	void ReloadTextSizes() {
		int selected = comboboxChatSize.GetCurrentItem();
		comboboxChatSize.ClearAll();
		for (int i = CHAT_SIZE_START; i <= 25; i++) {
			comboboxChatSize.AddItem("" + i);
		}
		comboboxChatSize.SetCurrentItem(OBLMarkerVisibilityManager.Get().GetChatSize() - CHAT_SIZE_START);
	}
	
	void ReloadPingMarkers(bool force = false) {
		TStringArray pingIconsArray2 = OBLMarkerVisibilityManager.Get().GetPingMarkerIcons();
		if (force || pingIconsArray.Count() != pingIconsArray2.Count()) {
			comboboxPingIcon.ClearAll();
			pingIconsArray = pingIconsArray2;
			foreach (string str : pingIconsArray2) {
				string displayname = str;
				int index1 = displayname.LastIndexOf("\\");
				int index2 = displayname.LastIndexOf("/");
				if (index1 < index2)
					index1 = index2;
				if (index1 > 0) {
					displayname = displayname.Substring(index1 + 1, displayname.Length() - index1 - 1);
				}
				index1 = displayname.LastIndexOf(".");
				if (index1 != -1) {
					displayname = displayname.Substring(0, index1);
				}
				if (displayname.Length() > 0) {
					string first = displayname[0] + "";
					first.ToUpper();
					displayname[0] = first;
				}
				comboboxPingIcon.AddItem(displayname);
			}
			comboboxPingIcon.SetCurrentItem(0);
			string currentIcon = OBLMarkerVisibilityManager.Get().GetPingMarkerIcon();
			int index = pingIconsArray.Find(currentIcon);
			if (index >= 0)
				comboboxPingIcon.SetCurrentItem(index);
			PingIconChanged();
		}
	}
	
	void PingIconChanged() {
		int pingIndex = comboboxPingIcon.GetCurrentItem();
		if (!pingIconsArray || pingIndex < 0 || pingIndex >= pingIconsArray.Count())
			return;
		string icon = pingIconsArray.Get(pingIndex);
		OBLMarkerVisibilityManager.Get().SetPingMarkerIcon(icon);
		imagePingIcon.LoadImageFile(0, icon);
		imagePingIcon.SetColor(OBLColorManager.Get().GetColor("Ping 3D Marker"));
	}
	
	override void OnHide() {
		super.OnHide();
		OBLColorManager.InvokeOnChanged();
		OBLPositionManager.InvokeOnChanged();
	}
	
	override void OnUpdateFrame() {
		colorPreview.SetColor(GetCurrentColor());
	}
	
	void LoadColor(int color) {
		int a = (color >> 24) & 0xFF;
		int r = (color >> 16) & 0xFF;
		int g = (color >> 8) & 0xFF;
		int b = color & 0xFF;
		sliderA.SetCurrent(a);
		sliderR.SetCurrent(r);
		sliderG.SetCurrent(g);
		sliderB.SetCurrent(b);
		editA.SetText("" + ((int) sliderA.GetCurrent()));
		editR.SetText("" + ((int) sliderR.GetCurrent()));
		editG.SetText("" + ((int) sliderG.GetCurrent()));
		editB.SetText("" + ((int) sliderB.GetCurrent()));
	}
	
	void SetCurrentColor() {
		int selectedItem = availableColors.GetSelectedRow();
		if (selectedItem < 0) {
			LoadColor(-1);
			return;
		}
		OBLColorManager.Get().SetColor(colorArray.Get(selectedItem), GetCurrentColor());
		imagePingIcon.SetColor(OBLColorManager.Get().GetColor("Ping 3D Marker"));
	}
	
	void ResetToDefaultColor() {
		int selectedItem = availableColors.GetSelectedRow();
		if (selectedItem < 0) {
			LoadColor(-1);
			return;
		}
		string colorStr = colorArray.Get(selectedItem);
		OBLColorManager.Get().ResetColorToDefault(colorStr);
		int color = OBLColorManager.Get().GetColor(colorStr);
		LoadColor(color);
	}
	
	void ResetToDefaultPosition() {
		OBLWidgetPosition position = GetCurrentPosition();
		if (!position) {
			return;
		}
		OBLPositionManager.Get().ResetPositionToDefault(position.param1);
		LoadPosition();
		SetInvertBoxes(position.param3);
	}
	
	int GetCurrentColor() {
		int a = sliderA.GetCurrent();
		int r = sliderR.GetCurrent();
		int g = sliderG.GetCurrent();
		int b = sliderB.GetCurrent();
		return ARGB(a,r,g,b);
	}
	
	void LoadPosition() {
		OBLWidgetPosition positionParam = GetCurrentPosition();
		if (!positionParam) {
			return;
		}
		vector pos = positionParam.param2;
		sliderX.SetCurrent(pos[0]);
		sliderY.SetCurrent(pos[1]);
		OnIconPreviewPositionChange();
	}
	#ifndef OBL_DISABLE_CHAT
	void OnChatSizeChanged() {
		int size = comboboxChatSize.GetCurrentItem() + CHAT_SIZE_START;
		OBLMarkerVisibilityManager.Get().chatSize = size;
		MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
		if (mission && mission.m_Chat) {
			mission.m_Chat.ResetTextSizes();
		}
	}
	#endif
	void OnIconPreviewPositionChange() {
		OBLWidgetPosition positionParam = GetCurrentPosition();
		if (!positionParam) {
			return;
		}
		int index = positionParam.param3;
		float posX = sliderX.GetCurrent();
		float posY = sliderY.GetCurrent();
		vector pos = Vector(posX, posY, 0);
		OBLWidgetUtils.SetWidgetPositionIndex(previewIcon, pos, index);
		OBLWidgetUtils.SetWidgetAlignmentIndex(previewIcon, index);
		OBLPositionManager.Get().SetPosition(positionsArray.Get(availableLayoutsList.GetSelectedRow()), pos);
	}
	
	void SetInvertBoxes(int index) {
		int value = OBLWidgetUtils.FromIndex(index);
		invertX.SetChecked(!OBLWidgetUtils.IsLeft(value));
		invertY.SetChecked(!OBLWidgetUtils.IsTop(value));
	}
	
	void SaveInvertBoxes() {
		int index = 0;
		if (invertX.IsChecked())
			index += 2;
		if (invertY.IsChecked())
			index += 6;
		OBLWidgetPosition positionParam = GetCurrentPosition();
		if (!positionParam) {
			return;
		}
		positionParam.param3 = index;
		OnIconPreviewPositionChange();
	}
	
	OBLWidgetPosition GetCurrentPosition() {
		int selectedItem = availableLayoutsList.GetSelectedRow();
		if (selectedItem < 0 || selectedItem >= OBLPositionManager.Get().positions.Count()) {
			return null;
		}
		return OBLPositionManager.Get().positions.Get(selectedItem);
	}
	
	override bool OnClick(Widget w) {
		if (super.OnClick(w))
			return true;
		if (w == toDefault) {
			ResetToDefaultColor();
			return true;
		} else if (w == btn_ResetAll) {
			OBLColorManager.Get().ResetAll();
			OBLPositionManager.Get().ResetAll();
			OBLLayoutConfig.ResetAll();
			OBLMarkerVisibilityManager.Get().ResetPingToDefault();
			MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
			if (mission)
				mission.SendOnlinePrivacyRPC();
			ReloadAll(true);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(OBLLayoutConfig.InvokeOnLayoutChanged);
			NotificationSystem.AddNotificationExtended(4.0, OBLTheme.NOTIFY_TITLE, "Усі налаштування скинуто до значень за замовчуванням!", OBLTheme.ICON_SUCCESS);
		} else if (w == btn_Save_Config) {
			OBLColorManager.Get().Save();
			OBLPositionManager.Get().Save();
			OBLLayoutConfig.Get().Save();
			OBLMarkerVisibilityManager.Get().Save();
			NotificationSystem.AddNotificationExtended(4.0, OBLTheme.NOTIFY_TITLE, "Налаштування збережено!", OBLTheme.ICON_SUCCESS);
		} else if (w == btn_Reload_From_Config) {
			OBLColorManager.Reload();
			OBLPositionManager.Reload();
			OBLLayoutConfig.Reload();
			OBLMarkerVisibilityManager.Get().ResetPingToLast();
			MissionGameplay missionReload = MissionGameplay.Cast(GetGame().GetMission());
			if (missionReload)
				missionReload.SendOnlinePrivacyRPC();
			ReloadAll(true);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(OBLLayoutConfig.InvokeOnLayoutChanged);
			NotificationSystem.AddNotificationExtended(4.0, OBLTheme.NOTIFY_TITLE, "Налаштування перезавантажено!", OBLTheme.ICON_SUCCESS);
		} else if (w == invertX || w == invertY) {
			if (w == invertX) {
				sliderX.SetCurrent(1.0 - sliderX.GetCurrent());
			} else {
				sliderY.SetCurrent(1.0 - sliderY.GetCurrent());
			}
			SaveInvertBoxes();
			LoadPosition();
			return true;
		} else if (w == toDefaultPosition) {
			ResetToDefaultPosition();
			return true;
		} else if (w == toDefaultStyle) {
			
		} else if (w == chckbx_Playerlist) {
			OBLMarkerVisibilityManager.Get().playerlistEnabled = !OBLMarkerVisibilityManager.Get().playerlistEnabled;
		} else if (w == chckbx_Compass) {
			OBLMarkerVisibilityManager.Get().compassEnabled = !OBLMarkerVisibilityManager.Get().compassEnabled;
		} else if (w == chckbx_ShowClantextures) {
			OBLMarkerVisibilityManager.Get().disableShowClantextures = !chckbx_ShowClantextures.IsChecked();
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(!OBLMarkerVisibilityManager.Get().disableShowClantextures);
			rpc.Send(null, 45113254, true);
		} else if (w == chckbx_ShowChatTag) {
			bool checked = chckbx_ShowChatTag.IsChecked();
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb && pb.GetOBLParty()) {
				pb.GetOBLParty().SendChatTagVisibilityRequest(checked);
			}
		} else if (w == comboPlayerlistStyleTemp) {
			OBLLayoutConfig.Get().SetPlayerlistLayout(comboPlayerlistStyleTemp.GetCurrentItem());
		} else if (w == chckbx_StreamerMode) {
			OBLLayoutConfig.Get().SetStreamerMode(chckbx_StreamerMode.IsChecked());
		}
		return false;
	}
	
	override bool OnItemSelected(Widget w, int row, int column) {
		if (super.OnItemSelected(w, row, column))
			return true;
		if (w == availableColors) {
			int selectedItem = availableColors.GetSelectedRow();
			if (selectedItem < 0) {
				LoadColor(-1);
				return true;
			}
			Param2<string, int> colorParam = OBLColorManager.Get().colorss.Get(selectedItem);
			if (!colorParam) {
				LoadColor(-1);
				return true;
			}
			LoadColor(colorParam.param2);
			return true;
		} else if (w == availableLayoutsList) {
			OBLWidgetPosition positionParam = GetCurrentPosition();
			if (!positionParam) {
				SetInvertBoxes(0);
				LoadPosition();
				return true;
			}
			SetInvertBoxes(positionParam.param3);
			LoadPosition();
			return true;
		} else if (w == availableLayoutsListStyles) {
			
		}
		return false;
	}
	
	override bool OnChange(Widget w) {
		if (super.OnChange(w))
			return true;
		bool changed = false;
		if (w == sliderA) {
			int a = sliderA.GetCurrent();
			editA.SetText(a.ToString());
			changed = true;
		} else if (w == sliderR) {
			int r = sliderR.GetCurrent();
			editR.SetText(r.ToString());
			changed = true;
		} else if (w == sliderG) {
			int g = sliderG.GetCurrent();
			editG.SetText(g.ToString());
			changed = true;
		} else if (w == sliderB) {
			int b = sliderB.GetCurrent();
			editB.SetText(b.ToString());
			changed = true;
		} else if (w == editA) {
			int txtInt = Math.Clamp(editA.GetText().ToInt(), 0, 255);
			sliderA.SetCurrent(txtInt);
			editA.SetText(txtInt.ToString());
			changed = true;
		} else if (w == editR) {
			txtInt = Math.Clamp(editR.GetText().ToInt(), 0, 255);
			sliderR.SetCurrent(txtInt);
			editR.SetText(txtInt.ToString());
			changed = true;
		} else if (w == editG) {
			txtInt = Math.Clamp(editG.GetText().ToInt(), 0, 255);
			sliderG.SetCurrent(txtInt);
			editG.SetText(txtInt.ToString());
			changed = true;
		} else if (w == editB) {
			txtInt = Math.Clamp(editB.GetText().ToInt(), 0, 255);
			sliderB.SetCurrent(txtInt);
			editB.SetText(txtInt.ToString());
			changed = true;
		} else if (w == sliderY || w == sliderX) {
			OnIconPreviewPositionChange();
		} else if (w == comboBoxStyle) {
			
		} else if (w == comboboxPingIcon) {
			PingIconChanged();
		} else if (w == comboboxChatSize) {
			#ifndef OBL_DISABLE_CHAT
			OnChatSizeChanged();
			#endif
		} else if (w == comboboxTeammate3DDistance) {
			OnTeammate3DDistanceChanged();
		}
		if (changed) {
			SetCurrentColor();
			return true;
		}
		return false;
	}
	
}
