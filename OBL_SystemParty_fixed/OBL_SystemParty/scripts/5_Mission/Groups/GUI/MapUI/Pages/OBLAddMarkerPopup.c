class OBLAddMarkerPopup {
	
	TextWidget addTitle;
	ImageWidget image_Icon;
	SliderWidget sliderR, sliderG, sliderB, sliderA;
	ButtonWidget btn_add_marker, btn_add_cancel, btn_add_delete, btn_clear_name;
	EditBoxWidget input_marker_name;
	XComboBoxWidget comboBoxImage, comboBoxVisibility;
	bool hasGroupVisibility = false;
	bool hasAdminVisibility = false;
	vector addPosition;

	ref array<ref OBLMarkerType> availableTypes = new array<ref OBLMarkerType>();
	
	ref OBLMarker edit_marker;
	
	Widget addPopup;
	OBLPartyUI parent;
	
	void StoreAllWidgetData(OBLDataSerializer data) {
		data.Write(new Param4<float, float, float, float>(sliderR.GetCurrent(), sliderG.GetCurrent(), sliderB.GetCurrent(), sliderA.GetCurrent()));
		data.Write(new Param1<string>(input_marker_name.GetText()));
		data.Write(new Param2<int, int>(comboBoxImage.GetCurrentItem(), comboBoxVisibility.GetCurrentItem()));
	}
	
	void RestoreAllWidgetData(OBLDataSerializer data) {
		Param4<float, float, float, float> colorParam = Param4<float, float, float, float>.Cast(data.Read());
		sliderR.SetCurrent(colorParam.param1);
		sliderG.SetCurrent(colorParam.param2);
		sliderB.SetCurrent(colorParam.param3);
		sliderA.SetCurrent(colorParam.param4);
		Param1<string> nameParam = Param1<string>.Cast(data.Read());
		input_marker_name.SetText(nameParam.param1);
		Param2<int, int> selectParam = Param2<int, int>.Cast(data.Read());
		comboBoxImage.SetCurrentItem(selectParam.param1);
		comboBoxVisibility.SetCurrentItem(selectParam.param2);
		UpdateIconImage();
	}
	
	void Init(OBLPartyUI parentUI) {
		parent = parentUI;
		addPopup = GetGame().GetWorkspace().CreateWidgets(OBLLayoutConfig.Get().GetCurrentLayout("Map Marker Add Popup"), parent.layoutRoot);
		addPopup.Show(false);
		
		btn_add_marker = ButtonWidget.Cast(addPopup.FindAnyWidget("btn_add_marker"));
		btn_add_cancel = ButtonWidget.Cast(addPopup.FindAnyWidget("btn_add_cancel"));
		btn_add_delete = ButtonWidget.Cast(addPopup.FindAnyWidget("btn_add_delete"));
		btn_clear_name = ButtonWidget.Cast(addPopup.FindAnyWidget("btn_clear_name"));
		
		comboBoxImage = XComboBoxWidget.Cast(addPopup.FindAnyWidget("comboBoxImage"));
		comboBoxVisibility = XComboBoxWidget.Cast(addPopup.FindAnyWidget("comboBoxVisibility"));
		
		input_marker_name = EditBoxWidget.Cast(addPopup.FindAnyWidget("input_marker_name"));
		sliderA = SliderWidget.Cast(addPopup.FindAnyWidget("sliderA"));
		sliderR = SliderWidget.Cast(addPopup.FindAnyWidget("sliderR"));
		sliderG = SliderWidget.Cast(addPopup.FindAnyWidget("sliderG"));
		sliderB = SliderWidget.Cast(addPopup.FindAnyWidget("sliderB"));
		image_Icon = ImageWidget.Cast(addPopup.FindAnyWidget("image_Icon"));
		
		foreach (string str : OBLPartyMainConfig.Get().availableIcons) {
			string displayname = str;
			int index = displayname.LastIndexOf("\\");
			int index2 = displayname.LastIndexOf("/");
			if (index < index2)
				index = index2;
			if (index > 0) {
				displayname = displayname.Substring(index + 1, displayname.Length() - index - 1);
			}
			index = displayname.LastIndexOf(".");
			if (index != -1) {
				displayname = displayname.Substring(0, index);
			}
			if (displayname.Length() > 0) {
				string first = displayname[0] + "";
				first.ToUpper();
				displayname[0] = first;
			}
			comboBoxImage.AddItem(displayname);
		}
		comboBoxImage.SetCurrentItem(0);
		UpdateIconImage();
		FillAvailableMarkerTypes();
	}

	
	void OnUpdateFrame() {
		SetIconColor();
	}
	
	void OnUpdateSlow() {
		
	}
	
	void SetIconColor() {
		image_Icon.SetColor(ARGB(sliderA.GetCurrent(), sliderR.GetCurrent(), sliderG.GetCurrent(), sliderB.GetCurrent()));
	}
	
	void OnGroupChanged() {
		FillAvailableMarkerTypes();
	}
	
	void AddMarkerButtonClicked() {
		int visibility = comboBoxVisibility.GetCurrentItem();
		if (visibility < 0 || visibility >= availableTypes.Count())
			return;
		if (!hasGroupVisibility)
			visibility += 1;
		if (visibility > 1 && !hasAdminVisibility)
			return;
		string name = input_marker_name.GetText();
		int iconIndex = comboBoxImage.GetCurrentItem();
		string icon = "";
		TStringArray icons = OBLPartyMainConfig.Get().availableIcons;
		if (iconIndex >= 0 && iconIndex < icons.Count())
			icon = icons.Get(iconIndex);
		int colorA = sliderA.GetCurrent();
		int colorR = sliderR.GetCurrent();
		int colorG = sliderG.GetCurrent();
		int colorB = sliderB.GetCurrent();
		if (edit_marker) {
			vector pos = edit_marker.position;
			RequestMarkerDelete(edit_marker);
			Hide();
			edit_marker = null;
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AddMarker, 1200, false, pos, visibility, name, icon, colorR, colorG, colorB, colorA);
		} else {
			AddMarker(addPosition, visibility, name, icon, colorR, colorG, colorB, colorA);
			Hide();
		}
	}
	
	void AddMarker(vector pos, int visibility, string name, string icon, int colorR, int colorG, int colorB, int colorA) {
		string printname = name + "";
		printname.Replace("%", "");
		OBLLogger.Debug("AddMarker: " + pos + " " + visibility + " " + printname + " " + icon);
		OBLMarker marker = new OBLMarker();
		OBLMarkerType type = OBLMarkerType.GROUP_MARKER;
		if (visibility == 0) {
			type = OBLMarkerType.GROUP_MARKER;
		} else if (visibility == 1) {
			type = OBLMarkerType.PRIVATE_MARKER;
		} else if (visibility == 2) {
			type = OBLMarkerType.SERVER_DYNAMIC;
		} else if (visibility == 3) {
			type = OBLMarkerType.SERVER_STATIC;
		}
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		if (pos[1] < 1)
			pos[1] = 1;
		marker.SetupMarker(type, name, icon, pos);
		marker.colorA = colorA;
		marker.colorR = colorR;
		marker.colorG = colorG;
		marker.colorB = colorB;
		if (type == OBLMarkerType.GROUP_MARKER) {
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb) {
				OBLParty grp = pb.GetOBLParty();
				if (grp) {
					grp.AddMarker(marker);
				}
			}
		} else if (type == OBLMarkerType.PRIVATE_MARKER) {
			OBLPrivateMarkerManager.Get().AddMarker(marker);
		} else if (type == OBLMarkerType.SERVER_STATIC || type == OBLMarkerType.SERVER_DYNAMIC) {
			OBLStaticMarkerManagerClient.Get().RequestGlobalMarkerAdd(marker);
		}
	}
	
	void UpdateIconImage() {
		int selected = comboBoxImage.GetCurrentItem();
		if (selected < 0 || selected >= OBLPartyMainConfig.Get().availableIcons.Count())
			return;
		string image = OBLPartyMainConfig.Get().availableIcons.Get(selected);
		image_Icon.LoadImageFile(0, image);
	}
	
	void ShowPopup(int x, int y, bool deleteMode = false, OBLMarker marker = null) {
		vector pos = Vector(x,y,0);
		addPosition = parent.mapWidget.ScreenToMap(pos);
		addPopup.Show(true);
		OBLLogger.Debug("Showing Add Popup " + x + " " + y + " MapPos: " + addPosition);
		btn_add_delete.Show(deleteMode);
		edit_marker = marker;
		if (deleteMode) {
			if (addTitle)
				addTitle.SetText("Редагувати маркер");
			int index = OBLPartyMainConfig.Get().availableIcons.Find(marker.icon);
			if (index == -1)
				index = 0;
			comboBoxImage.SetCurrentItem(index);
			int typeIndex = availableTypes.Find(marker.type);
			if (typeIndex == -1)
				typeIndex = 0;
			comboBoxVisibility.SetCurrentItem(typeIndex);
			sliderA.SetCurrent(marker.colorA);
			sliderR.SetCurrent(marker.colorR);
			sliderG.SetCurrent(marker.colorG);
			sliderB.SetCurrent(marker.colorB);
			input_marker_name.SetText(marker.name);
			UpdateIconImage();
			SetIconColor();
		} else {
			if (addTitle)
				addTitle.SetText("Додати маркер");
		}
	}
	
	bool OnClick(Widget w) {
		if (w == btn_add_marker) {
			AddMarkerButtonClicked();
			return true;
		} else if (w == btn_add_cancel) {
			Hide();
			return true;
		} else if (w == btn_add_delete) {			
			RequestMarkerDelete(edit_marker);
			Hide();
			return true;
		} else if (w == btn_clear_name) {
			input_marker_name.SetText("");
			return true;
		}
		return false;
	}

	void Hide() {
		if (addPopup)
			addPopup.Show(false);
	}
	
	bool OnChange(Widget w) {
		if (w == comboBoxVisibility) {
		
			return true;
		} else if (w == comboBoxImage) {
			UpdateIconImage();
			return true;
		}
		return false;
	}
	
	void FillAvailableMarkerTypes() {
		comboBoxVisibility.ClearAll();
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		availableTypes.Clear();
		if (pb && pb.GetOBLParty()) {
			comboBoxVisibility.AddItem("Група");
			hasGroupVisibility = true;
			availableTypes.Insert(OBLMarkerType.GROUP_MARKER);
		} else {
			hasGroupVisibility = false;
		}
		comboBoxVisibility.AddItem("Приватний");
		availableTypes.Insert(OBLMarkerType.PRIVATE_MARKER);
		if (MissionGameplay.groupAdmin) {
			comboBoxVisibility.AddItem("Глобальний (тимчасовий)");
			comboBoxVisibility.AddItem("Глобальний (постійний)");
			availableTypes.Insert(OBLMarkerType.SERVER_DYNAMIC);
			availableTypes.Insert(OBLMarkerType.SERVER_STATIC);
			hasAdminVisibility = true;
		} else {
			hasAdminVisibility = false;
		}
	}
	
	void DeleteMarkerUnderMouse() {
		int x, y;
		GetMousePos(x,y);
		OBLLogger.Debug("DeleteMarkerUnderMouse. Pos: " + x + "," + y);
		vector mousePos = Vector(x + 10,y + 10,0);
		vector mapPos = parent.mapWidget.ScreenToMap(mousePos);
		OBLLogger.Debug("MapPos: " + mapPos);
		OBLMarker marker = FindMarkerInRadius(mapPos);
		RequestMarkerDelete(marker);
	}
	
	void RequestMarkerDelete(OBLMarker marker) {
		if (marker) {
			string printname = marker.name + "";
			printname.Replace("%", "");
			OBLLogger.Debug("Removing Marker: " + printname + " " + marker.icon);
			if (marker.type == OBLMarkerType.PRIVATE_MARKER) {
				OBLPrivateMarkerManager.Get().RemoveMarker(marker);
			} else if (marker.type == OBLMarkerType.GROUP_MARKER) {
				PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
				if (!pb || !pb.GetOBLParty())
					return;
				OBLParty grp = pb.GetOBLParty();
				grp.RemoveMarker(marker);
			} else if (marker.type == OBLMarkerType.SERVER_STATIC || marker.type == OBLMarkerType.SERVER_DYNAMIC) {
				OBLStaticMarkerManagerClient.Get().RequestGlobalMarkerRemove(marker.uid);
			}
		}
	}
	
	OBLMarker FindMarkerInRadius(vector center, float radius = 500) {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb)
			return null;
		OBLParty grp = pb.GetOBLParty();
		OBLMarker bestMarker = null;
		float bestDist = radius + 1;
		if (grp) {
			foreach (OBLMarker marker : grp.markers) {
				if (!OBLMarkerVisibilityManager.Get().IsMapVisible(marker.uid, marker.type))
					continue;
				float dist = Math.Sqrt((marker.position[0] - center[0]) * (marker.position[0] - center[0]) + (marker.position[2] - center[2]) * (marker.position[2] - center[2]));
				if (dist < bestDist) {
					OBLLogger.Debug("New Best Marker: " + marker + " OLD: " + bestMarker + " Dist: " + dist + " OLD: " + bestDist);
					bestMarker = marker;
					bestDist = dist;
				}
			}
		}
		foreach (OBLMarker marker2 : OBLPrivateMarkerManager.Get().privateMarkers) {
			if (!OBLMarkerVisibilityManager.Get().IsMapVisible(marker2.uid, marker2.type))
				continue;
			dist = Math.Sqrt((marker2.position[0] - center[0]) * (marker2.position[0] - center[0]) + (marker2.position[2] - center[2]) * (marker2.position[2] - center[2]));
			if (dist < bestDist) {
				OBLLogger.Debug("New Best Marker: " + marker2 + " OLD: " + bestMarker + " Dist: " + dist + " OLD: " + bestDist);
				bestMarker = marker2;
				bestDist = dist;
			}
		}
		if (hasAdminVisibility) {
			foreach (OBLServerMarker serverMark1 : OBLStaticMarkerManagerClient.Get().staticMarkers) {
				if (!OBLMarkerVisibilityManager.Get().IsMapVisible(serverMark1.uid, serverMark1.type))
					continue;
				dist = Math.Sqrt((serverMark1.position[0] - center[0]) * (serverMark1.position[0] - center[0]) + (serverMark1.position[2] - center[2]) * (serverMark1.position[2] - center[2]));
				if (dist < bestDist) {
					OBLLogger.Debug("New Best Marker: " + serverMark1 + " OLD: " + bestMarker + " Dist: " + dist + " OLD: " + bestDist);
					bestMarker = serverMark1;
					bestDist = dist;
				}
			}
		}
		OBLLogger.Debug("Best Marker: " + bestMarker + " Dist: " + bestDist);
		return bestMarker;
	}

}
