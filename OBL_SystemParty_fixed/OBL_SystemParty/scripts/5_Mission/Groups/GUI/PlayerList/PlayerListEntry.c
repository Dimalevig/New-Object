class OBLPlayerListEntry {

	// OBL FIX: was a strong ref back to the list (reference cycle)
	OBLPlayerList parentList;
	ref OBLPartyMember member;
	
	Widget mainWidget;
	ProgressBarWidget healthbar;
	TextWidget playername, distance;
	Widget borderWidget;
	
	float lastHealth;
	bool lastDowned;
	
	void ~OBLPlayerListEntry(){
		if (mainWidget)
			mainWidget.Unlink();
	}
	
	void Init(OBLPlayerList parent, OBLPartyMember groupMember) {
		parentList = parent;
		mainWidget = GetGame().GetWorkspace().CreateWidgets(GetPlayerListLayout(), parent.listWidget);
		if (!mainWidget) {
			InitMember(groupMember);
			return;
		}
		healthbar = ProgressBarWidget.Cast(mainWidget.FindAnyWidget("healthbar"));
		playername = TextWidget.Cast(mainWidget.FindAnyWidget("playername"));
		distance = TextWidget.Cast(mainWidget.FindAnyWidget("distance"));
		borderWidget = mainWidget.FindAnyWidget("borderWidget");
		Show(false);
		InitMember(groupMember);
	}
	
	string GetPlayerListLayout() {
		return OBLLayoutConfig.Get().GetCurrentLayout("Player List");
	}
	
	void InitMember(OBLPartyMember groupMember) {
		member = groupMember;
	}
	
	void UpdateWidget() {
		if (!member || !member.online) {
			Show(false);
			return;
		}
		OBLLogger.Debug("Update Playerlits Widget " + member.name);
		Show(true);
		if (healthbar) {
			healthbar.SetCurrent(member.health);
			healthbar.SetColor(GetColor(member.health));
		}
		if (playername) {
			string danger = member.GetDangerText();
			if (danger != "")
				playername.SetText(member.name + "  —  " + danger);
			else
				playername.SetText(member.name);
		}
		lastHealth = member.health;
		lastDowned = member.downed;
		if (borderWidget) {
			int borderColor = OBLColorManager.Get().GetColor("Playerlist entry border");
			borderWidget.SetColor(borderColor);
		}
	}
	
	void UpdateDistance() {
		if (distance) {
			if (OBLPartyMainConfig.Get().enablePlayerListDistance && member && member.steamid != MissionBaseWorld.mySteamid) {
				distance.Show(true);
				vector pos = GetGame().GetCurrentCameraPosition();
				float dist = vector.Distance(member.position, pos);
				if (dist < 1000) {
					distance.SetText(" " + ((int) dist) + "m");
				} else {
					float km = ((float) ((int) (dist / 100))) / 10;
					distance.SetText(" " + km + "km");
				}
			} else {
				distance.Show(false);
			}
		}
	}
	
	int GetColor(float health) {
		int colorFull = GetHealthFullColor();
		int colorZero = GetHealthEmptyColor();
		float colorPartFull = health / 100.0;
		float colorPartZero = 1.0 - colorPartFull;
		int a = ((colorFull >> 24) & 0xFF) * colorPartFull + ((colorZero >> 24) & 0xFF) * colorPartZero;
		int r = ((colorFull >> 16) & 0xFF) * colorPartFull + ((colorZero >> 16) & 0xFF) * colorPartZero;
		int g = ((colorFull >> 8) & 0xFF) * colorPartFull + ((colorZero >> 8) & 0xFF) * colorPartZero;
		int b = ((colorFull) & 0xFF) * colorPartFull + ((colorZero) & 0xFF) * colorPartZero;
		return ARGB(a,r,g,b);
	}
	
	int GetHealthEmptyColor() {
		return OBLColorManager.Get().GetColor("Playerlist entry zero health");
	}
	
	int GetHealthFullColor() {
		return OBLColorManager.Get().GetColor("Playerlist entry full health");
	}
	
	void Show(bool b) {
		if (mainWidget)
			mainWidget.Show(b);
	}

}