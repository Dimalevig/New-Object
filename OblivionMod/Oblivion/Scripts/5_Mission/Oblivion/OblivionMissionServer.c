modded class MissionServer
{
	protected ref OblivionCapturePoints   m_OblivionCapturePoints;
	protected ref OblivionPlaytimeRewards m_OblivionPlaytimeRewards;

	override void OnInit()
	{
		super.OnInit();
		OblivionSettings.Get();

		m_OblivionCapturePoints = new OblivionCapturePoints();
		m_OblivionCapturePoints.Start();

		m_OblivionPlaytimeRewards = new OblivionPlaytimeRewards();
		m_OblivionPlaytimeRewards.Start();
	}

	override void OnMissionFinish()
	{
		if (m_OblivionPlaytimeRewards)
			m_OblivionPlaytimeRewards.Save();

		super.OnMissionFinish();
	}

	// Запасний шлях для команд: якщо клієнт не перехопив команду і вона пішла в чат — ловимо її тут.
	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);

		if (eventTypeId != ChatMessageEventTypeID)
			return;

		ChatMessageEventParams chat;
		if (!Class.CastTo(chat, params) || !OblivionIsChatCommand(chat.param3))
			return;

		PlayerBase player = OblivionNotify.FindPlayerByName(chat.param2);
		if (player)
			player.OblivionRunChatCommand(chat.param3);
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!player || !identity)
			return;

		ScriptRPC rpc = new ScriptRPC();
		OblivionSettings.Get().WriteSync(rpc);
		rpc.Send(player, OBLIVION_RPC_SETTINGS, true, identity);

		OblivionPlaytimeRewards.OnConnect(player);
	}

	// Combat log: вихід у бою — персонаж стоїть у світі довше.
	override void OnClientDisconnectedEvent(PlayerIdentity identity, PlayerBase player, int logoutTime, bool authFailed)
	{
		OblivionCombatLogSettings s = OblivionSettings.Get().CombatLog;
		if (s.Enabled && !s.KillOnLeave && player && player.OblivionIsInCombat())
			logoutTime = Math.Max(logoutTime, s.StayInWorldSeconds);

		super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
	}

	// Combat log: вихід у бою — персонаж помирає, тіло з лутом лишається.
	override bool ShouldPlayerBeKilled(PlayerBase player)
	{
		if (super.ShouldPlayerBeKilled(player))
			return true;

		OblivionCombatLogSettings s = OblivionSettings.Get().CombatLog;
		return s.Enabled && s.KillOnLeave && player && player.OblivionIsInCombat();
	}
}
