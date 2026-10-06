modded class MissionGameplay {
	
	ref OBLPartyUI openedMapUI = null;
	static bool groupAdmin = false;
	int lastPingUID = -1;
	
	// Global Chat
	#ifndef OBL_DISABLE_CHAT
	int currentChannel = 0;
	bool muted = false;
	bool setDefaultChannel = false;
	ref array<ref ChannelCfg> channels = new array<ref ChannelCfg>();
	#endif
	ref OBLCompassHud compassHud;
	bool randomInitialized = false;
	int serverToClientTimeOffset = 0;

	void MissionGameplay() {		
		Print("[Init] --- MissionGameplay ---");

		if (GetGame().IsClient()) {
			Print("---------" + "[CLIENT]" + "---------");			
			OBLPartyMainConfig.Delete();
			OBLFilePlus.Init();
			OBLLogger.Init();
			OBLLayoutConfig.Get();
		}

		GetDayZGame().Event_OnRPC.Insert(RPC_OBL);
		OBLLayoutConfig.Event_StreamerModeChanged.Insert(OnStreamerModeChange);

		if (GetGame().IsMultiplayer()) {
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_STEAMID, new Param1<bool>(true), true);
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_INVITE_CODE, new Param1<bool>(true), true);
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_ADMIN_STATUS, new Param1<bool>(true), true);
			SendOnlinePrivacyRPC();
		}
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OBLMarker.UpdateAllMarkersSlow, 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateTimeAcceleration, 60000, true);
		if (PlayerBase.bxd_player_list)
			PlayerBase.bxd_player_list.Clear();		
		#ifndef OBL_DISABLE_CHAT
		setDefaultChannel = false;
		#endif

		randomInitialized = false;
		openedMapUI = new OBLPartyUI();
		
		OBLLogger.Debug("Finish MissionGameplay");
	}

	override void OnInit() {
		super.OnInit();
		OBLTextLengthCalculator.Get();
		SendTextureRPC();
		GetGame().RPCSingleParam(null, 37454343, new Param1<bool>(true), true);
		GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_SERVER_TIME, new Param1<bool>(true), true);
	}
	
	void ~MissionGameplay() {
		GetDayZGame().Event_OnRPC.Remove(RPC_OBL);
		OBLLayoutConfig.Event_StreamerModeChanged.Remove(OnStreamerModeChange);
		if (openedMapUI)
			delete openedMapUI;
		openedMapUI = null;
		OBLStaticMarkerManagerClient.Delete();
		OBLPartyPermissions.Delete();
		OBLPrivateMarkerManager.Delete();
		OBLPlayerList.Delete();
		OBLMarkerVisibilityManager.Delete();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OBLMarker.UpdateAllMarkersSlow);
		if (compassHud)
			delete compassHud;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateTimeAcceleration);
		OBLTextLengthCalculator.Delete();
		if (GetGame().IsClient()) {
			OBLFilePlus.DeleteJson(OBLPartyConstants.SAVE_SUFFIX_MAIN_CONFIG);
		}
	}
	
	#ifdef THKOTH
	override void RPCKOTHUpdateZoneStatus( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		super.RPCKOTHUpdateZoneStatus(type, ctx, sender, target);
		if (!openedMapUI)
			return;
		if (!OBLPartyMainConfig.Get().enableKOTHMarkers)
			return;
		openedMapUI.AddCustomMarkersOnMapOpen();
	}
	
	void AddKOTHMarker(MapWidget mapWidget, OBLMapMarkerManager mgr) {
		if (!OBLPartyMainConfig.Get().enableKOTHMarkers)
			return;
		if (ZoneCenter == vector.Zero)
			return;
		mapWidget.AddUserMark(ZoneCenter, ZoneName, ARGB(220, 0, 86, 130), "KingOfTheHillAssets\\gui\\images\\Flag.paa");
		mgr.AddCircleNonScaling(ZoneCenter, CaptureRadius, ARGB(220, 0, 86, 130), 5489);
	}
	
	#endif
	
	void SendTextureRPC() {
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(!OBLMarkerVisibilityManager.Get().disableShowClantextures);
		rpc.Send(null, 45113254, true);
	}
	
	int lastingameTime = 0;
	static int acceleration = 0;
	
	void OnStreamerModeChange(bool enabled) {
		OBLMarker.streamerMode = enabled;
		OBLMarker.UpdateAllMarkersSlow();
	}
	
	void UpdateTimeAcceleration() {
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		int time = hour * 60 + minute;
		while (lastingameTime > time)
			time += 1440;
		if (lastingameTime == 0) {
			lastingameTime = time % 1440;
			return;
		}
		acceleration = time - lastingameTime;
		lastingameTime = time % 1440;
	}
	
	void OnMainConfigReceived() {
		OBLLogger.Debug("OnMainConfigReceived start");
		OBLLogger.Debug("Getting Static Markers...");
		OBLStaticMarkerManagerClient.Get();
		OBLLogger.Debug("Getting Group Permissions...");
		OBLPartyPermissions.Get();
		OBLLogger.Debug("Loading Private Markers....");
		OBLPrivateMarkerManager.Get(GetServerString());
		OBLLogger.Debug("Loading Marker Visibility Manager...");
		OBLMarkerVisibilityManager.Get();
		OBLLogger.Debug("Finished Loading Marker Visibility Manager");
		if (OBLPartyMainConfig.Get().enablePlayerList) {
			OBLLogger.Debug("Loading Player List...");
			OBLPlayerList.Get();
		} else {
			OBLPlayerList.Delete();
		}
		if (OBLPartyMainConfig.Get().enableCompassHud) {
			OBLLogger.Debug("Creating Compass Hud");
			compassHud = new OBLCompassHud();
			compassHud.InitWidgets();
		} else if (compassHud) {
			delete compassHud;
		}
		OBLLogger.Debug("OnMainConfigReceived finish");
		
		//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SendTestBEMessage, 30000, false);
	}
	
	void SendTestBEMessage() {
		ChatMessageEventParams message = new ChatMessageEventParams(CCAdmin, "Дreykwood", "Test MEssage", "");
		m_Chat.Add(message);
	}
	
	string GetServerString() {
		GetServersResultRow info = OnlineServices.GetCurrentServerInfo();
		MenuData menu_data = g_Game.GetMenuData();
		if (info) {
			return info.m_HostIp + ":" + info.m_HostPort.ToString();
		} else if (menu_data && menu_data.GetLastPlayedCharacter() != GameConstants.DEFAULT_CHARACTER_MENU_ID) {
			int char_id = menu_data.GetLastPlayedCharacter();
			int port;
			string address;
			
			menu_data.GetLastServerAddress(char_id,address);
			port = menu_data.GetLastServerPort(char_id);
			return address + ":" + port;
		}
		return "";
	}
	
	override void OnGroupChanged() {
		super.OnGroupChanged();
		if (openedMapUI) {
			openedMapUI.OnGroupChanged();
		}
		if (OBLPartyMainConfig.Get().enablePlayerList) {
			OBLPlayerList.Get().OnGroupChanged();
		}
		#ifndef OBL_DISABLE_CHAT
		UpdateChannel();
		#endif
	}
	
	void RPC_OBL(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
		if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_MAIN) {
			OBLPartyMainConfig.Get().RPC_OBL(sender, ctx);
			OnMainConfigReceived();
			OBLPartyMainConfig.Get().PrintMarkerConfigEntries();
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS) {
			OBLStaticMarkerManagerClient.Get().RPC_OBL(sender, ctx);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_PERMISSIONS) {
			OBLPartyPermissions.Get().RPC_OBL(sender, ctx);
		} else if (rpc_type == OBLPartyRPCs.GROUP_SYNC) {
			PlayerBase pbSync = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pbSync)
				return;
			bool hasGroup = false;
			if (!ctx.Read(hasGroup))
				return;
			if (!hasGroup) {
				pbSync.SetOBLParty(null);
				return;
			}
			OBLParty syncGroup = new OBLParty();
			if (!syncGroup.ReadFromCtx(ctx)) {
				OBLLogger.Debug("Failed to receive Group from Mission RPC !");
				return;
			}
			OBLLogger.Debug("Successfully received Group from Mission RPC");
			pbSync.SetOBLParty(syncGroup);
			syncGroup.InitMarkers();
		} else if (rpc_type == OBLPartyRPCs.SHOP_LIST_SYNC) {
			OBLShopClient.Get().OnListSync(ctx);
		} else if (rpc_type == OBLPartyRPCs.SHOP_CLAIM_RESULT) {
			OBLShopClient.Get().OnClaimResult(ctx);
		} else if (rpc_type == OBLPartyRPCs.GROUP_RPC) {
			OBLLogger.Debug("Receivd Group RPC");
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pb || !pb.GetOBLParty())
				return;
			int type = 0;
			if (!ctx.Read(type))
				return;
			OBLLogger.Debug("Received Group RPC and Found Group. " + type);
			pb.GetOBLParty().OnRPCClient(type, ctx);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADD_CLIENT_MARKER) {
			string name_, icon_, creatorId;
			vector position_;
			int color_;
			if (!ctx.Read(name_) || !ctx.Read(icon_) || !ctx.Read(position_) || !ctx.Read(color_) || !ctx.Read(creatorId))
				return;

			AddClientMarkerFromServer(name_, icon_, position_, color_, creatorId);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_STEAMID) {
			Param1<string> strParam;
			if (!ctx.Read(strParam)) {
				OBLLogger.Debug("Failed to receive own Steamid !");
				return;
			}
			mySteamid = strParam.param1;
			OBLLogger.Debug("Received own Steamid: " + mySteamid);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_INVITE_CODE) {
			Param1<string> codeParam;
			if (!ctx.Read(codeParam)) {
				OBLLogger.Debug("Failed to receive own invite code !");
				return;
			}
			myInviteCode = codeParam.param1;
			OBLLogger.Debug("Received own invite code: " + myInviteCode);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_ONLINE_PRIVACY_LIST) {
			OBLOnlinePrivacyManager.ReadHiddenSteamids(ctx);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_ADMIN_STATUS) {
			Param1<bool> adminParam;
			if (!ctx.Read(adminParam)) {
				OBLLogger.Debug("Failed to Read Admin RPC");
				return;
			}
			groupAdmin = adminParam.param1;
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_LIST || rpc_type == OBLPartyRPCs.GROUP_ADMIN_LIST_SINGLE || rpc_type == OBLPartyRPCs.GROUP_ADMIN_DELETE) {
			OBLAdminPage adminP = GetAdminPage();
			if (!adminP)
				return;
			if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_LIST)
				adminP.OnAllGroupsReceived(ctx);
			else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_LIST_SINGLE)
				adminP.OnGroupUpdateReceived(ctx);
			else
				adminP.OnGroupDeleteReceived(ctx);
		} else if (rpc_type == OBLPartyRPCs.GROUP_ADMIN_FLAGS) {
			int count2;
			if (!ctx.Read(count2))
				return;
			array<ref Param2<ref vector, string>> arr = new array<ref Param2<ref vector, string>>();
			OBLLogger.Debug("Reading " + count2 + " Flag Positions from the Server ...");
			for (int i2 = 0; i2 < count2; i2++) {
				bool exists = false;
				if (!ctx.Read(exists))
					return;
				if (!exists)
					continue;
				float x,y,z;
				string name2;
				if (!ctx.Read(x) || !ctx.Read(y) || !ctx.Read(z) || !ctx.Read(name2))
					return;
				arr.Insert(new Param2<ref vector, string>(Vector(x,y,z), name2));
			}
			OBLLogger.Debug("Read " + arr.Count() + " valid Flag Positions");
			adminP = GetAdminPage();
			if (!adminP)
				return;
			adminP.SetFlagPositions(arr);
			adminP.ShowTerritorryCheckbox();
		}
		#ifndef OBL_DISABLE_CHAT
		else if (rpc_type == OBLPartyRPCs.OBL_GLOBAL_CHAT) {
			
			int channel;
			string name, message, extra, prefix;
			int prefixColor;
			string groupPrefix;
			int channelColor;
			if (!ctx.Read(channel) || !ctx.Read(name) || !ctx.Read(message) || !ctx.Read(extra) || !ctx.Read(prefix) || !ctx.Read(prefixColor) || !ctx.Read(groupPrefix) || !ctx.Read(channelColor))
				return;
			message = message.Substring(1, message.Length() - 1);
			//OBLLogger.Debug("Chat Message Received: " + channel + " " + name + " " + message + " " + extra);
			m_Chat.AddOBLChat( channel, name, message, extra, prefix, prefixColor, groupPrefix, channelColor);
			
		} else if (rpc_type == OBLPartyRPCs.OBL_GLOBAL_MUTELIST) {
			Param1<bool> muteParam;
			if (!ctx.Read(muteParam))
				return;
			bool old = muted;
			muted = muteParam.param1;
		} else if (rpc_type == OBLPartyRPCs.OBL_GLOBAL_CHANNELS) {
			OBLLogger.Debug("Received Channel RPC");
			int count;
			if (!ctx.Read(count))
				return;
			channels.Clear();
			int def = 0;
			for (int i = 0; i < count; i++) {
				ChannelCfg cfgchannel = new ChannelCfg();
				if (!cfgchannel.ReadFromCtx(ctx))
					return;
				channels.Insert(cfgchannel);
				if (cfgchannel.defaultChannel)
					def = i;
			}
			OBLLogger.Debug("Received " + channels.Count() + " Channels.");
			if (!setDefaultChannel) {
				currentChannel = def;
				UpdateChannel();
				setDefaultChannel = true;
			}
		}
		#endif
		else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_SERVER_TIME) {
			Param3<int, int, int> timeParam;
			if (!ctx.Read(timeParam))
				return;
			int hour,min,sec;
			GetHourMinuteSecond(hour,min,sec);
			int timestampServer = timeParam.param1 * 3600 + timeParam.param2 * 60 + timeParam.param3;
			int timestampClient = hour * 3600 + min * 60 + sec;
			serverToClientTimeOffset = timestampServer - timestampClient;
			OBLLogger.Debug("Servertime offset received: " + serverToClientTimeOffset);
		}
	}

	void AddClientMarkerFromServer(string name, string icon, vector position, int color, string creatorId = "Server") {
		OBLLogger.Debug("Received Client Marker from Server: " + name + " Icon: " + icon + " color: " + color);
		// OBL FIX: death markers ("PM") piled up forever — keep only the latest one
		if (creatorId == "PM")
			OBLPrivateMarkerManager.Get().RemoveMarkersLike(name, icon);
		OBLMarker marker = new OBLMarker();
		marker.SetupMarker(OBLMarkerType.PRIVATE_MARKER, name, icon, position);
		marker.SetColorInt(color);
		OBLPrivateMarkerManager.Get().AddMarker(marker);
	}
	
	bool GetServerInfoOBL(out string ip, out int port) {
		MenuData menu_data = g_Game.GetMenuData();
		GetServersResultRow info = OnlineServices.GetCurrentServerInfo();
		
		if (info) {
			ip = info.m_HostIp;
			port = info.m_HostPort;
			return true;
		} else if (menu_data && menu_data.GetLastPlayedCharacter() != GameConstants.DEFAULT_CHARACTER_MENU_ID) {
			int char_id = menu_data.GetLastPlayedCharacter();
			string address,name;
			
			menu_data.GetLastServerAddress(char_id,address);
			port = menu_data.GetLastServerPort(char_id);
			ip = address;
			return true;
		}
		return false;
	}
	
	OBLAdminPage GetAdminPage() {
		if (!openedMapUI)
			return null;
		OBLPartyPage adminPage = openedMapUI.GetPageByName("Адмін");
		if (!adminPage)
			return null;
		OBLAdminPage adminP;
		Class.CastTo(adminP, adminPage);
		return adminP;
	}
	
	const int PING_TIMEOUT = 1000;
	int lastPing = 0;
	
	override void OnUpdate(float timeslice) {
		bool openMapInGroupMenu = GetUApi() && GetUApi().GetInputByName("IGroupOpenMapGroup").LocalPress();
		if (GetUApi() && GetUApi().GetInputByName("IGroupOpenMap").LocalPress() || openMapInGroupMenu) {
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb && !pb.IsUnconscious()) {
				if (GetGame().GetUIManager().GetMenu() && GetGame().GetUIManager().GetMenu() == openedMapUI && !openMapInGroupMenu) {
					if (!openedMapUI.typing)
					openedMapUI.HideMenu();
				} else if (!GetGame().GetUIManager().GetMenu() && !GetGame().GetUIManager().IsCursorVisible()) {
					if (!openedMapUI)
						openedMapUI = new OBLPartyUI();
					openedMapUI.ShowMenu();
					if (openMapInGroupMenu) {
						openedMapUI.OpenGroupPage();
					}
				}
			}
		}
		else if (GetUApi() && GetUApi().GetInputByName("IGroupTacticalPing").LocalPress()) {
			if (IsNoMenuOpen()) {
				int now = GetGame().GetTime();
				if (now - PING_TIMEOUT > lastPing) {
					lastPing = now;
					ClearPing();
					AddPing();
				}
			}
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupTacticalPingClear").LocalPress()) {
			if (IsNoMenuOpen())
				ClearPing();
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupAcceptInvite").LocalPress()) {
			if (IsNoMenuOpen())
				AcceptGroupInvite();
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupToggleCompass").LocalPress()) {
			if (IsNoMenuOpen())
				ToggleCompass();
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupTogglePlayerList").LocalPress()) {
			if (IsNoMenuOpen())
				TogglePlayerList();
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupToggleVisibility").LocalPress()) {
			if (!GetGame().GetUIManager().GetMenu() && !GetGame().GetUIManager().IsCursorVisible()) {
				OBLMarkerVisibilityManager.Get().GetNextState();
				OBLMarker.UpdateAllMarkersSlow();
				string state = OBLMarkerVisibilityManager.Get().GetCurrentStateName();
				pb = PlayerBase.Cast(GetGame().GetPlayer());
				if (pb)
					pb.MessageImportant("Стан маркерів: " + state);
			}
		} else if (GetUApi() && GetUApi().GetInputByName("IGroupDeleteMarker").LocalPress()) {
			if (GetGame().GetUIManager().GetMenu() && GetGame().GetUIManager().GetMenu() == openedMapUI) {
				if (!openedMapUI.typing)
					openedMapUI.addPopup.DeleteMarkerUnderMouse();
			}
		}
		#ifndef OBL_DISABLE_CHAT
		if (GetUApi() && IsNoMenuOpen()) {
			UAInput switchChatChannel = GetUApi().GetInputByName("ISwitchChatChannel");
			if (switchChatChannel && switchChatChannel.LocalPress()) {
				if (channels.Count() > 1) {
					SwitchNextChannel();
					NotificationSystem.AddNotificationExtended(1.0, OBLTheme.NOTIFY_TITLE_CHAT, "Канал: " + GetCurrentChannel(), OBLTheme.ICON_SUCCESS);
					//GetGame().Chat("Channel: " + GetCurrentChannel(), "colorAction");
				}
			}
		}
		#endif
		
		if (compassHud) {
			compassHud.UpdateHud();
		}
		if (OBLPartyMainConfig.Get().enablePlayerList) {
			OBLPlayerList.Get().UpdateVisibility();
		}
		OBLMarker.UpdateAllMarkers();
		#ifndef OBL_DISABLE_CHAT
		if (m_Chat) {
			m_Chat.UpdateChatVisibility();
		}
		#endif
		//*/
		super.OnUpdate(timeslice);
	}
	
	bool IsNoMenuOpen() {
		return GetGame() && GetGame().GetUIManager() && !GetGame().GetUIManager().GetMenu();
	}

	void SendOnlinePrivacyRPC() {
		if (!GetGame() || !GetGame().IsMultiplayer())
			return;
		GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_ONLINE_PRIVACY, new Param1<bool>(OBLLayoutConfig.Get().hideOnlineStatus), true);
	}
	
	void ToggleCompass() {
		if (OBLPartyMainConfig.Get().enableCompassHud) {
			OBLMarkerVisibilityManager.Get().compassEnabled = !OBLMarkerVisibilityManager.Get().compassEnabled;
			OBLMarkerVisibilityManager.Get().Save();
		}
	}
	
	void TogglePlayerList() {
		if (OBLPartyMainConfig.Get().enablePlayerList) {
			OBLMarkerVisibilityManager.Get().playerlistEnabled = !OBLMarkerVisibilityManager.Get().playerlistEnabled;
			OBLMarkerVisibilityManager.Get().Save();
		}
	}
	
	void AcceptGroupInvite() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb)
			return;
		// OBL FIX: pressing the key with no pending invite gave no feedback at all
		if (lastInvite == "") {
			NotificationSystem.AddNotificationExtended(4.0, OBLTheme.NOTIFY_TITLE, "У вас немає активного запрошення до групи.", OBLTheme.ICON_ERROR);
			return;
		}
		Param1<string> lastInviteParam = new Param1<string>(lastInvite);
		OBLLogger.Debug("Accepted Invite for " + lastInvite);
		GetGame().RPCSingleParam(null, OBLPartyRPCs.GROUP_ACCEPT_INVITE, lastInviteParam, true);
		// OBL FIX: consume the invite so the key cannot be spammed
		lastInvite = "";
	}
	
	void AddPing() {
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		OBLParty grp = pb.GetOBLParty();
		string name;
		GetGame().GetPlayerName(name);
		OBLMarker marker = new OBLMarker();
		vector camPos = GetGame().GetCurrentCameraPosition();
		vector camDir = GetGame().GetCurrentCameraDirection().Normalized() * 2000;
		Object hitObj;
		vector hitPos, hitNormal;
		float fraction;
		PhxInteractionLayers layers = PhxInteractionLayers.ITEM_SMALL | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.VEHICLE_NOTERRAIN | PhxInteractionLayers.BUILDING | PhxInteractionLayers.CHARACTER | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ROADWAY | PhxInteractionLayers.FIREGEOM | PhxInteractionLayers.DOOR | PhxInteractionLayers.WATERLAYER | PhxInteractionLayers.TERRAIN | PhxInteractionLayers.FENCE | PhxInteractionLayers.AI;
		DayZPhysics.RayCastBullet(camPos, camPos + camDir, layers, GetGame().GetPlayer(), hitObj, hitPos, hitNormal, fraction);
		marker.SetupMarker(OBLMarkerType.GROUP_PING, name, "", hitPos);
		marker.colorR = 255;
		marker.colorG = 255;
		marker.colorB = 0;
		marker.icon = "OBL_SystemParty/gui/icons/ping.paa";
		OBLPartyMember myMarker = pb.GetMyGroupMarker();
		if (!myMarker)
			return;
		marker.currentSubgroup = myMarker.currentSubgroup;
		grp.AddMarker(marker);
		lastPingUID = marker.uid;
	}
	
	void ClearPing() {
		OBLLogger.Debug("Removing Last Ping: " + lastPingUID);
		if (lastPingUID == -1)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetOBLParty())
			return;
		OBLParty grp = pb.GetOBLParty();
		OBLMarker marker = grp.FindPingMarkerByUID(lastPingUID);
		if (marker) {
			grp.RemoveMarker(marker);
			string printname = marker.name + "";
			printname.Replace("%", "");
			OBLLogger.Debug("Removing Marker Ping: " + printname);
		}
	}
	#ifndef OBL_DISABLE_CHAT
	void DisplayVoiceLevels(bool b) {
		m_VoiceLevels.Show(b);
	}
	
	string GetCurrentChannel() {
		if (currentChannel < 0 || currentChannel >= channels.Count())
			return "Невідомо";
		ChannelCfg cfgchannel = channels.Get(currentChannel);
		if (!cfgchannel)
			return "Невідомо";
		return cfgchannel.channelName;
	}
	
	void SendChatMessage(string message) {
		if (message.Length() == 0)
			return;
		if (muted && message[0] != "!") {
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (!pb)
				return;
			pb.MessageImportant("Ви заглушені!");
			return;
		}
		ChannelCfg cfgchannel = null;
		if (currentChannel >= 0 && currentChannel < channels.Count())
			cfgchannel = channels.Get(currentChannel);
		if (!cfgchannel) {
			OBLLogger.Debug("Failed to get Channel Config for Channel " + currentChannel);
			return;
		}
		if (message[0] == "!") {
			GetGame().ChatPlayer(message);
			message = "+" + message;
		} else if (cfgchannel.directChannel) {
			GetGame().ChatPlayer(message);
			message = "+" + message;
		} else {
			message = "+" + message;
			GetGame().ChatPlayer(message);
		}
		if(GetGame().IsMultiplayer()) {
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(currentChannel);
			rpc.Write(message);
			rpc.Send(NULL, OBLPartyRPCs.OBL_GLOBAL_CHAT, true);
			OBLLogger.Debug("Sending Multiplayer Chat Message");
		} else {
			string name;
			GetGame().GetPlayerName( name );
			ChatMessageEventParams chat_params = new ChatMessageEventParams( CCDirect, name, message, "" );
			m_Chat.Add( chat_params );
			OBLLogger.Debug("Sending Singleplayer Chatmessage");
		}
	}
	
	void SwitchNextChannel() {
		currentChannel++;
		UpdateChannel();
	}
	
	void UpdateChannel() {
		if (!channels || channels.Count() <= 0)
			return;
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		bool inGroup = (pb != null && pb.GetOBLParty() != null);
		// OBL FIX: was recursive and overflowed the stack when every channel was a group channel and the player had no group
		for (int tries = 0; tries < channels.Count(); tries++) {
			currentChannel = currentChannel % channels.Count();
			ChannelCfg cfgchannel = channels.Get(currentChannel);
			if (cfgchannel && (!cfgchannel.groupChannel || inGroup))
				return;
			currentChannel++;
		}
		currentChannel = currentChannel % channels.Count();
	}
	#endif

}
