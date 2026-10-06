class OBLInfoPage : OBLPartyPage {
	const int INFO_BUTTON_COUNT = 2;
	
	ref array<ButtonWidget> leftButtons = new array<ButtonWidget>();
	ref array<TextWidget> leftButtonsText = new array<TextWidget>();
	ref TStringArray buttonLinks = new TStringArray();
	
	ButtonWidget btnCpyPlayerCoords;
	
	TextWidget txt_0, txt_1, txt_2, txt_3_0, txt_3_1, txt_4;
	TextWidget desc_txt0, desc_txt1, desc_txt2, desc_txt3, desc_txt4;
	Widget mousePosPanel;
	
	void OBLInfoPage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);
	}
	void ~OBLInfoPage() {
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
	}

	override bool InitPage(OBLPartyUI parentUI) {
		return super.InitPage(parentUI, 0, 0, "інфо", false);
	}
	
	override void InitMainWidget() {
		OBLLogger.Debug("InitMainWidget Info Page");
		for (int i = 0; i < INFO_BUTTON_COUNT; i++) {
			leftButtons.Insert(ButtonWidget.Cast(rootWidget.FindAnyWidget("button" + i)));
			leftButtonsText.Insert(TextWidget.Cast(rootWidget.FindAnyWidget("buttonsubtext" + i)));
		}
		btnCpyPlayerCoords = ButtonWidget.Cast(rootWidget.FindAnyWidget("btnCpyPlayerCoords"));
		txt_0 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_0"));
		txt_1 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_1"));
		txt_2 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_2"));
		txt_3_0 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_3_0"));
		txt_3_1 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_3_1"));
		txt_4 = TextWidget.Cast(rootWidget.FindAnyWidget("txt_4"));
		desc_txt0 = TextWidget.Cast(rootWidget.FindAnyWidget("desc_txt0"));
		desc_txt1 = TextWidget.Cast(rootWidget.FindAnyWidget("desc_txt1"));
		desc_txt2 = TextWidget.Cast(rootWidget.FindAnyWidget("desc_txt2"));
		desc_txt3 = TextWidget.Cast(rootWidget.FindAnyWidget("desc_txt3"));
		desc_txt4 = TextWidget.Cast(rootWidget.FindAnyWidget("desc_txt4"));
		mousePosPanel = rootWidget.FindAnyWidget("mousePosPanel");
		
		Widget modcreator = rootWidget.FindAnyWidget("modcreator");
		if (modcreator && OBLPartyMainConfig.Get().disableInfoPanelModCreatorMention) {
			modcreator.Show(false);
		}
		OBLLogger.Debug("Initialized Buttons: " + leftButtons.Count());
		SetButtonContent();
		UpdateInfoCounter();
		if (btnCpyPlayerCoords && !OBLPartyMainConfig.Get().enableInfoPanelCursorCoordinates)
			btnCpyPlayerCoords.Show(false);
		OnStreamerModeChange(OBLLayoutConfig.Get().streamerModeEnabled);
	}
	
	override bool OnClick(Widget w) {
		for (int i = 0; i < leftButtons.Count(); i++) {
			ButtonWidget btn = leftButtons.Get(i);
			if (btn && w == btn) {
				if (i < buttonLinks.Count()) {
					string lnk = buttonLinks.Get(i);
					if (lnk.Length() > 0)
						GetGame().OpenURL(lnk);
				}
				return true;
			}
		}
		if (w == btnCpyPlayerCoords && GetGame().GetPlayer()) {
			vector pos = GetGame().GetPlayer().GetPosition();
			string posStr = "" + pos[0] + " " + pos[1] + " " + pos[2];
			GetGame().CopyToClipboard(posStr);
			NotificationSystem.AddNotificationExtended(4.0, "Система груп", "Координати гравця скопійовано", "set:ccgui_enforce image:MapUserMarker"); 
		}
		return false;
	}
	
	override void OnUpdateSlow() {
		UpdateInfoCounter();
	}
	
	override void OnUpdateFrame() {
		SetCoordinatesField();
	}
	
	void SetCoordinatesField() {
		if (!txt_3_0 || !txt_3_1)
			return;
		if (!OBLPartyMainConfig.Get().enableInfoPanelCursorCoordinates) {
			txt_3_0.Show(false);
			txt_3_1.Show(false);
			if (desc_txt3)
				desc_txt3.Show(false);
			if (mousePosPanel)
				mousePosPanel.Show(false);
			return;
		}
		if (!parent || !parent.mapWidget)
			return;
		int x, y;
		GetMousePos(x,y);
		vector mousePos = Vector(x,y,0);
		vector worldPos = parent.mapWidget.ScreenToMap(mousePos);
		float x1 = worldPos[0];
		float x2 = worldPos[2];
		x = x1;
		int z = x2;
		txt_3_0.SetText("X:" + x);
		txt_3_1.SetText("Z:" + z);
	}
	
	void UpdateSurvivorCount() {
		if (txt_0 && desc_txt0) {
			if (!OBLPartyMainConfig.Get().enableInfoPanelSurvivorCount) {
				txt_0.Show(false);
				desc_txt0.Show(false);
				return;
			}
			int cnt = GetSurvivorCount();
			if (cnt == 1)
				txt_0.SetText("" + cnt);
			else
				txt_0.SetText("" + cnt);
		}
	}
	
	int GetSurvivorCount() {
		if (!GetGame().IsMultiplayer())
			return 1;
		if (GetGame().IsClient()) {
			if (ClientData.m_PlayerList && ClientData.m_PlayerList.m_PlayerList) {
				int visibleCount = 0;
				for (int i = 0; i < ClientData.m_PlayerList.m_PlayerList.Count(); i++) {
					SyncPlayer syncPlayer = ClientData.m_PlayerList.m_PlayerList.Get(i);
					if (syncPlayer && !OBLOnlinePrivacyManager.IsHidden(syncPlayer.m_UID))
						visibleCount++;
				}
				return visibleCount;
			}
		} else if (GetGame().IsServer()) {
			ref array<Man> players = new array<Man>();
			GetGame().GetPlayers(players);
			int count = 0;
			foreach (Man player : players) {
				if (player && player.GetIdentity())
					count++;
			}
			return count;
		}
		return -1;
	}

	void UpdateInfoCounter() {
		UpdateIngameTime();
		UpdateServerTime();
		UpdateSurvivorCount();
		UpdateInviteCode();
	}

	void UpdateInviteCode() {
		if (!txt_4 || !desc_txt4)
			return;
		string inviteCode = MissionBaseWorld.myInviteCode;
		if (inviteCode == "")
			inviteCode = "-";
		txt_4.SetText(inviteCode);
	}

	void SetButtonContent() {
		if (leftButtons.Count() == 0)
			return;
		buttonLinks.Clear();
		array<ref OBLButtonConfig> btns = OBLPartyMainConfig.Get().buttonConfig;
		for (int i = 0; i < INFO_BUTTON_COUNT; i++) {
			buttonLinks.Insert("");
			if (btns.Count() <= i) {
				if (leftButtons.Get(i))
					leftButtons.Get(i).Show(false);
				continue;
			}
			OBLButtonConfig cfg = btns.Get(i);
			if (!leftButtons.Get(i) || !leftButtonsText.Get(i))
				continue;
			if (!cfg || cfg.buttonName == "") {
				leftButtons.Get(i).Show(false);
				continue;
			}
			leftButtons.Get(i).Show(!OBLLayoutConfig.Get().streamerModeEnabled);
			leftButtonsText.Get(i).Show(!OBLLayoutConfig.Get().streamerModeEnabled);
			leftButtons.Get(i).SetText(cfg.buttonName);
			leftButtonsText.Get(i).SetText(cfg.subtext);
			buttonLinks.Set(i, cfg.link);
		}
	}
	
	void OnStreamerModeChange(bool enabled) {
		bool show = !enabled;
		OBLLogger.Debug("OnStreamerModeChange InfoPage: " + enabled);
		foreach (ButtonWidget btn : leftButtons) {
			if (btn)
				btn.Show(show);
		}
		foreach (TextWidget txt : leftButtonsText) {
			if (txt)
				txt.Show(show);
		}
		SetButtonContent();
		if (txt_0)
			txt_0.Show(show);
		if (txt_1)
			txt_1.Show(show);
		if (txt_2)
			txt_2.Show(show);
		if (txt_3_0)
			txt_3_0.Show(show);
		if (txt_3_1)
			txt_3_1.Show(show);
		if (desc_txt0)
			desc_txt0.Show(show);
		if (desc_txt1)
			desc_txt1.Show(show);
		if (desc_txt2)
			desc_txt2.Show(show);
		if (desc_txt3)
			desc_txt3.Show(show);
		if (txt_4)
			txt_4.Show(show);
		if (desc_txt4)
			desc_txt4.Show(show);
		if (mousePosPanel)
			mousePosPanel.Show(show);
		UpdateInfoCounter();
		SetCoordinatesField();
	}
	
	void UpdateIngameTime() {
		if (txt_1 && desc_txt1) {
			if (!OBLPartyMainConfig.Get().enableInfoPanelIngameTime) {
				txt_1.Show(false);
				desc_txt1.Show(false);
				return;
			}
			if (!GetGame() || !GetGame().GetWorld())
				return;
			int year, month, day, hour, minute;
			GetGame().GetWorld().GetDate(year, month, day, hour, minute);
			string hourStr = "" + hour;
			if (hour < 10)
				hourStr = "0" + hour;
			string minuteStr = "" + minute;
			if (minute < 10)
				minuteStr = "0" + minute;
			string timeStr = "" + hourStr + ":" + minuteStr;
			txt_1.SetText(timeStr);
		}
	}
	
	void UpdateServerTime() {
		if (txt_2 && desc_txt2) {
			if (!OBLPartyMainConfig.Get().enableInfoPanelRealTime) {
				txt_2.Show(false);
				desc_txt2.Show(false);
				return;
			}
			if (!GetGame() || !GetGame().GetMission())
				return;
			MissionGameplay mission = MissionGameplay.Cast(g_Game.GetMission());
			int offset = mission.serverToClientTimeOffset;
			if (offset < 0)
				offset += 24 * 3600;
			OBLDate now = OBLDate.Init(false);
			int hours = now.m_hour;
			int min = now.m_min;
			min += offset / 60;
			while (min >= 60) {
				min -= 60;
				hours += 1;
			}
			if (hours >= 24)
				hours -= 24;
			string timeStr = "";
			if (hours < 10) {
				timeStr = "0" + hours;
			} else {
				timeStr = "" + hours;
			}
			timeStr += ":";
			if (min < 10) {
				timeStr += "0" + min;
			} else {
				timeStr += "" + min;
			}
			txt_2.SetText(timeStr);
		}
	}
	
}
