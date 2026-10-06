#ifndef OBL_DISABLE_CHAT
modded class Chat {

	bool addedPrefix = false;
	Widget rootChatWidget;
	float prefixLength = 0;
	
	bool added = false;
	
	private string nextPrefix, nextGroupPrefix;
	
	override void Init(Widget root_widget) {
		OBLPositionManager.Event_OnPositionChange.Remove(UpdatePosition);
		Widget other = GetGame().GetWorkspace().CreateWidgets(GetMainChatLayout(), null);
		if (other) {
			Widget other2 = other.FindAnyWidget("ChatFrameWidget");
			if (other2)
				root_widget = other2;
		}
		super.Init(root_widget);
		rootChatWidget = root_widget;
		OBLPositionManager.Event_OnPositionChange.Insert(UpdatePosition);
		UpdatePosition();
	}
	
	string GetMainChatLayout() {
		return "OBL_SystemParty/gui/layouts/chatmain.layout";
	}
	
	void ~Chat() {
		OBLPositionManager.Event_OnPositionChange.Remove(UpdatePosition);
	}
	
	void AddOBLChat(int channel, string name, string message, string extra, string prefix, int prefixColor, string groupPrefix, int channelColor) {
		SetNextPrefix(prefix, prefixColor, groupPrefix, channelColor);
		addedPrefix = true;
		OBLTextLengthCalculator calc = OBLTextLengthCalculator.Get();
		int size = OBLMarkerVisibilityManager.Get().GetChatSize();
		prefixLength = calc.GetTextLength(groupPrefix + " " + prefix, size);
		Add(new ChatMessageEventParams(channel, name, message, extra));
		if (added) {
			prefixLength = 0;
			addedPrefix = false;
			ChatLine.currentColor = ARGB(255, 255, 255, 255);
		} else {
			SetNextPrefix("", 0, "", 0);
		}
	}
	
	void UpdatePosition() {
		vector pos = OBLPositionManager.Get().GetPosition("Chat");
		int index = OBLPositionManager.Get().GetIndex("Chat");
		OBLWidgetUtils.SetWidgetAlignmentIndex(rootChatWidget, index);
		OBLWidgetUtils.SetWidgetPositionIndex(rootChatWidget, pos, index);
	}
	
	void UpdateChatVisibility() {
		if (!rootChatWidget)
			return;
		IngameHud hud = IngameHud.Cast(GetGame().GetMission().GetHud());
		if (hud) {
			bool visible = hud.OBLIsHudVisible() && GetGame().GetPlayer() && GetGame().GetPlayer().IsAlive() && !GetGame().GetPlayer().IsUnconscious();
			rootChatWidget.Show(visible);
		}
	}
	
	override void Add(ChatMessageEventParams params)
	{
		added = false;
		string message = params.param3;
		if (message.Length() == 0)
			return;
		if (message[0] == "+" || message[0] == "!")
			return;
		if (message[0] == "2") {
			params.param3 = message.Substring(1, message.Length() - 1);
		}
		
		OBLTextLengthCalculator calc = OBLTextLengthCalculator.Get();
		
		int size = OBLMarkerVisibilityManager.Get().GetChatSize();
		float name_lenght = calc.GetTextLength(params.param2, size);
		float text_lenght = calc.GetTextLength(params.param3, size);
		float total_lenght = text_lenght + name_lenght + prefixLength;
		int channel =  params.param1;

		if( channel & CCSystem || channel & CCBattlEye) //TODO separate battleye bellow
 		{
			if( g_Game.GetProfileOption( EDayZProfilesOptions.GAME_MESSAGES ) )
				return;
 		}
		//TODO add battleye filter to options
		/*else if( channel & CCBattlEye ) 
		{
			if( g_Game.GetProfileOption( EDayZProfilesOptions.BATTLEYE_MESSAGES ) )
				return;
		}*/
		else if( channel & CCAdmin )
		{
			if( g_Game.GetProfileOption( EDayZProfilesOptions.ADMIN_MESSAGES ) )
				return;
		}
		else if( channel & CCDirect || channel & CCMegaphone || channel & CCTransmitter || channel & CCPublicAddressSystem ) 
		{
			if( g_Game.GetProfileOption( EDayZProfilesOptions.PLAYER_MESSAGES ) )
				return;
		}
		else if( channel == 4096 ) 
		{
			if( g_Game.GetProfileOption( EDayZProfilesOptions.PLAYER_MESSAGES ) )
				return;
		}
		bool first = true;
		float max_length = 0.4;
		if (total_lenght > max_length)
		{
			string preMessage = "";
			if (addedPrefix) {
				preMessage = nextPrefix + " " + nextGroupPrefix + " ";
			}
			preMessage = preMessage + params.param2;
			float preLength = calc.GetTextLength(preMessage, size);
			TStringArray parts = new TStringArray();
			params.param3.Split(" ", parts);
			int i = 0;
			
			ref ChatMessageEventParams tmp = new ChatMessageEventParams(params.param1, params.param2, "", params.param4);
			
			while (i < parts.Count())
			{
				string mess = "";
				// OBL FIX: bounds check must come first (parts[i] was read past the end)
				while (i < parts.Count() && calc.GetTextLength(mess + " " + parts[i], size) + preLength < max_length) {
					mess = mess + " " + parts[i];
					i++;
				}
				if (mess.Length() <= 0) {
					string part = parts[i];
					int l = 1;
					for (int a = 0; a < part.Length(); a++) {
						if (calc.GetTextLength(part.Substring(0, a + 1), size) + preLength >= max_length) {
							l = a;
							break;
						}
					}
					// OBL FIX: l == 0 never shortened the word -> endless loop
					if (l < 1)
						l = 1;
					mess = part.Substring(0, l);
					parts[i] = part.Substring(l, part.Length() - l);
				}
				tmp.param3 = mess;
				preLength = 0;
				
				if (!addedPrefix)
					SetNextPrefix("", 0, "", ARGB(255, 255, 255, 255));
				else if (!first)
					SetNextPrefix("", 0, "", ChatLine.currentColor);
				first = false;
				AddInternal(tmp);
				added = true;
				
				tmp.param2 = "";
			}
		}
		else
		{
			if (!addedPrefix) {
				SetNextPrefix("", 0, "", ARGB(255, 255, 255, 255));
			}
			AddInternal(params);
			added = true;
		}
		addedPrefix = false;
	}
	
	void ResetTextSizes() {
		int size = OBLMarkerVisibilityManager.Get().GetChatSize();
		foreach (ChatLine line : m_Lines) {
			line.m_NameTag.SetTextExactSize(size);
			line.m_NameWidget.SetTextExactSize(size);
			line.m_TextWidget.SetTextExactSize(size);
			line.m_GroupTag.SetTextExactSize(size);
		}
	}
	
	void SetNextPrefix(string prefix, int prefixColor, string groupPrefix, int channelColor) {
		ChatLine line = m_Lines.Get((m_LastLine + 1) % m_Lines.Count());
		int size = OBLMarkerVisibilityManager.Get().GetChatSize();
		line.m_NameTag.SetTextExactSize(size);
		line.m_NameTag.SetColor(prefixColor);
		line.m_NameTag.SetText(prefix);
		line.m_GroupTag.SetText(groupPrefix);
		ChatLine.currentColor = channelColor;
		
		nextPrefix = prefix;
		nextGroupPrefix = groupPrefix;
	}
	
}
#endif
