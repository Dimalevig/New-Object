// Команди мода (/contract ...) не йдуть у загальний чат, а надсилаються серверу.
modded class ChatInputMenu
{
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (finished && m_edit_box)
		{
			string text = m_edit_box.GetText();
			if (OblivionIsChatCommand(text))
			{
				PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
				if (player)
					GetGame().RPCSingleParam(player, OBLIVION_RPC_COMMAND, new Param1<string>(text), true);
				m_edit_box.SetText("");
			}
		}
		return super.OnChange(w, x, y, finished);
	}
}
