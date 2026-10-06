modded class MissionServer {
	static bool g_GameShuttingDown = false;

	void MissionServer() {
		Print("[Init] --- Group System Loading Configs ---");

		CreateMainConfigDir();
		OBLLogger.Init();
		OBLPartyMainConfig.Get();
		OBLPartyPermissions.Get();
		OBLStaticMarkerManager.Get();
		OBLPartyManager.Get();
		OBLShopManager.Get();
		GetDayZGame().Event_OnRPC.Insert(RPC_OBL);
		#ifndef OBL_DISABLE_CHAT
		GetMuteConfig();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateMuteList, 60000, true);
		#endif
		OBLLogger.Info("Group System Config Loading finished");
	}
	
	void ~MissionServer() {
		OBLPartyMainConfig.Delete();
		OBLPartyPermissions.Delete();
		OBLStaticMarkerManager.Delete();
		OBLPartyManager.Delete();
		GetDayZGame().Event_OnRPC.Remove(RPC_OBL);
		#ifndef OBL_DISABLE_CHAT
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateMuteList);
		#endif
	}

	override void OnMissionFinish() {
        g_GameShuttingDown = true;
        super.OnMissionFinish();
    }
	
	int GetOnlinePlayerCount() {
		ref array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		int count = 0;
		foreach (Man player : players) {
			if (player && player.GetIdentity())
				count++;
		}
		return count;
	}
	
	void CreateMainConfigDir() {
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
	}
	
	override void RPC_OBL(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
		if (g_GameShuttingDown)
        	return;
		
		super.RPC_OBL(sender, target, rpc_type, ctx);
		
		if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_MAIN) {
			OBLPartyMainConfig.Get().RPC_OBL(sender, ctx);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS || rpc_type == OBLPartyRPCs.CONFIG_GLOBAL_MARKER_ADD || rpc_type == OBLPartyRPCs.CONFIG_GLOBAL_MARKER_REMOVE) {
			OBLStaticMarkerManager.Get().RPC_OBL(sender, ctx, rpc_type);
		}  else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_PERMISSIONS) {
			OBLPartyPermissions.Get().RPC_OBL(sender, ctx);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_STEAMID) {
			if (!sender)
				return;
			string steamid = sender.GetPlainId();
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_STEAMID, new Param1<string>(steamid), true, sender);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_INVITE_CODE) {
			if (!sender)
				return;
			string inviteCode = OBLParty.GetOrCreateOBLInviteCode(sender);
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_INVITE_CODE, new Param1<string>(inviteCode), true, sender);
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_ONLINE_PRIVACY) {
			if (!sender)
				return;
			Param1<bool> privacyParam;
			if (!ctx.Read(privacyParam))
				return;
			OBLParty.SetOBLOnlinePrivacy(sender.GetPlainId(), privacyParam.param1);
			SendOBLOnlinePrivacyList();
		} else if (rpc_type == OBLPartyRPCs.CONFIG_SYNC_ADMIN_STATUS) {
			if (!sender)
				return;
			steamid = sender.GetPlainId();
			bool admin = OBLPartyMainConfig.Get().adminSteamids.Find(steamid) != -1;
			Param1<bool> adminParam = new Param1<bool>(admin);
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_ADMIN_STATUS, adminParam, true, sender);
		} else if (rpc_type == OBLPartyRPCs.OBL_GLOBAL_CHAT && sender) {
			#ifndef OBL_DISABLE_CHAT
			OnChatRPC(sender, ctx);
			#endif
		}
	}

	// коди запрошення для списку онлайн-гравців: новачку — усі, решті — лише код новачка
	void SendInviteCodes(PlayerIdentity joined) {
		if (!joined)
			return;
		TStringArray hidden = OBLParty.GetOBLHiddenOnlineSteamids();
		array<PlayerIdentity> identities = new array<PlayerIdentity>();
		GetGame().GetPlayerIndentities(identities);
		ScriptRPC full = new ScriptRPC();
		int count = 0;
		foreach (PlayerIdentity ident : identities) {
			if (ident && hidden.Find(ident.GetPlainId()) == -1)
				count++;
		}
		full.Write(count);
		foreach (PlayerIdentity ident2 : identities) {
			if (!ident2 || hidden.Find(ident2.GetPlainId()) != -1)
				continue;
			full.Write(ident2.GetPlainId());
			full.Write(OBLParty.GetOrCreateOBLInviteCode(ident2));
		}
		full.Send(null, OBLPartyRPCs.CONFIG_SYNC_INVITE_CODES, true, joined);
		if (hidden.Find(joined.GetPlainId()) != -1)
			return;
		ScriptRPC single = new ScriptRPC();
		single.Write(1);
		single.Write(joined.GetPlainId());
		single.Write(OBLParty.GetOrCreateOBLInviteCode(joined));
		foreach (PlayerIdentity other : identities) {
			if (other && other != joined)
				single.Send(null, OBLPartyRPCs.CONFIG_SYNC_INVITE_CODES, true, other);
		}
	}
	
	void SendOBLOnlinePrivacyList(PlayerIdentity target = null) {
		TStringArray hiddenSteamids = OBLParty.GetOBLHiddenOnlineSteamids();
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(hiddenSteamids.Count());
		for (int i = 0; i < hiddenSteamids.Count(); i++) {
			rpc.Write(hiddenSteamids.Get(i));
		}
		if (target) {
			rpc.Send(null, OBLPartyRPCs.CONFIG_SYNC_ONLINE_PRIVACY_LIST, true, target);
		} else {
			rpc.Send(null, OBLPartyRPCs.CONFIG_SYNC_ONLINE_PRIVACY_LIST, true);
		}
	}
	
	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity) {
		super.InvokeOnConnect(player, identity);
		if (!player || !identity)
			return;

		OBLParty.GetOrCreateOBLInviteCode(identity);
		player.InitGroupServer(identity);
		SendOBLOnlinePrivacyList(identity);
		SendInviteCodes(identity);

		#ifndef OBL_DISABLE_CHAT
		// Synchronize mute/chat state only for the joining player.
		// Avoid SendMuteList() here because it broadcasts to every online player.
		MuteConfig muteConfig = GetMuteConfig();
		if (muteConfig) {
			muteConfig.UpdateList();
			GetGame().RPCSingleParam(null, OBLPartyRPCs.OBL_GLOBAL_MUTELIST, new Param1<bool>(false), true, identity);
			if (muteConfig.IsMuted(identity.GetPlainId()))
				muteConfig.SendMute(identity);
			muteConfig.SendChatList(identity);
		}
		#endif
	}
	/*
	override void OnClientRespawnEvent( PlayerIdentity identity, PlayerBase player ) {
		super.OnClientRespawnEvent( identity, player );
		player.InitGroupServerRespawn(identity);
	}*/

	// CHAT
	#ifndef OBL_DISABLE_CHAT
	
	ref map<string, ref TStringSet> muteVotes = new map<string, ref TStringSet>();
	const int OBL_CHAT_MAX_LENGTH = 256;
	
	// раз на хвилину: лише знімаємо прострочені мути (шлемо тільки змінам).
	// Список каналів більше не розсилається всім щохвилини — гравець отримує його при вході
	void UpdateMuteList() {
		GetMuteConfig().SendMuteList();
	}
	
	void OnChatRPC(PlayerIdentity sender, ParamsReadContext ctx) {
		int channel = 0;
		string message = "";
		OBLLogger.Debug("Received Chat RPC");
		if (!ctx.Read(channel) || !ctx.Read(message))
			return;
		string name = sender.GetName();
		// OBL FIX: cap message size (client limit can be bypassed)
		if (message.Length() > OBL_CHAT_MAX_LENGTH)
			message = message.Substring(0, OBL_CHAT_MAX_LENGTH);
		// the client always prefixes "+"; an empty message is just that prefix
		if (message.Length() < 2 || message[0] != "+")
			return;
		int length = message.Length();
		if (message.Length() >= 2 && message[0] == "+" && message[1] == "!") {
			OBLLogger.Debug("trying to read Chat command");
			string cmd = message.Substring(2, message.Length() - 2);
			TStringArray args = new TStringArray();
			cmd.Split(" ", args);
			if (args.Count() > 0) {
				cmd = args.Get(0);
				args.RemoveOrdered(0);
				OnChatCommand(sender, cmd, args);
			}
			return;
		}
		// OBL FIX: mute was only enforced by the client
		MuteConfig muteCfg = GetMuteConfig();
		muteCfg.UpdateList();
		if (muteCfg.IsMuted(sender.GetPlainId()) && !muteCfg.IsAdmin(sender.GetPlainId())) {
			SendSimpleChatMessage(sender, "Вас вимкнено в чаті.");
			return;
		}
		//OBLLogger.Debug("Chat Message. Channel: " + channel + " Player: " + sender.GetPlainId() + " (" + sender.GetName() + "): " + message);
		GetGame().AdminLog("Chat Message. Channel: " + channel + " Player: " + sender.GetPlainId() + " (" + sender.GetName() + "): " + message);
		if (GetMuteConfig().enabledBadWordsCensor) {
			bool block = GetMuteConfig().blockBadWordContainingMessages;
			string messageLower = message + "";
			messageLower.ToLower();
			TStringArray badWords = GetMuteConfig().badWords;
			foreach (string badWord : badWords) {
				badWord.ToLower();
				int index = messageLower.IndexOf(badWord);
				if (index != -1) {
					if (block && GetMuteConfig().badWordsBlockedMessage.Length() > 0) {
						SendSimpleChatMessage(sender, GetMuteConfig().badWordsBlockedMessage);
					}
					if (GetMuteConfig().badWordsMuteTime > 0) {
						MuteChatPlayer(sender, GetMuteConfig().badWordsMuteTime);
						SendSimpleChatMessage(sender, "Вас вимкнено в чаті на " + GetMuteConfig().badWordsMuteTime + " хв");
					}
					if (block)
						return;
				}
				while (index != -1) {
					int wordEnd = index + badWord.Length();
					for (int i = 0; i < badWord.Length(); i++) {
						message[index + i] = "*";
					}
					index = messageLower.IndexOfFrom(wordEnd, badWord);
				}
				
			}
		}
		
		ScriptRPC rpc = new ScriptRPC();
		// OBL FIX: channel index comes from the client
		ChannelCfg cfg = null;
		if (channel >= 0 && channel < GetMuteConfig().channels.Count())
			cfg = GetMuteConfig().channels.Get(channel);
		int channelColor = 0;
		bool globalC = false;
		bool groupC = false;
			if (cfg && cfg.muted) {
				string steamid = sender.GetPlainId();
				if (!GetMuteConfig().IsAdmin(steamid)) {
					string messageErr = cfg.channelName + " зараз вимкнено!";
					SendSimpleChatMessage(sender, messageErr);
					return;
				}
		}
		if (cfg) {
			if (cfg.directChannel) // Direct
				return;
			channelColor = cfg.channelColor.GetColorARGB();
			globalC = cfg.globalChannel;
			groupC = cfg.groupChannel;
			channel = 4096;
		}
		rpc.Write(channel);
		rpc.Write(name);
		rpc.Write(message);
		rpc.Write("");
		PrefixGroup grp = GetMuteConfig().GetPrefixForSteamid(sender.GetPlainId());
		string prefix = "";
		string groupPrefix = "";
		int color = ARGB(255, 255, 255, 255);
		if (grp) {
			prefix = grp.prefix;
			color = grp.GetColor();
		}
		if (GetMuteConfig().displayGroupTagsInfrontOfName) {
			PlayerBase pb = PlayerBase.GetPlayerByIdentity(sender);
			if (pb && pb.GetOBLParty() && pb.GetOBLParty().showTagInChat) {
				groupPrefix = "[" + pb.GetOBLParty().shortname + "] ";
			}
		}
		rpc.Write(prefix);
		rpc.Write(color);
		rpc.Write(groupPrefix);
		rpc.Write(channelColor);
		
		if (globalC) { // Global
			OBLLogger.Debug("Sending To Global Chat");
			rpc.Send(NULL, OBLPartyRPCs.OBL_GLOBAL_CHAT, true);
		} else if (groupC) { // Group
			pb = PlayerBase.GetPlayerByIdentity(sender);
			if (!pb || !pb.GetOBLParty())
				return;
			OBLLogger.Debug("Sending To Group Chat");
			pb.GetOBLParty().SendRPCToGroupMembers(rpc, OBLPartyRPCs.OBL_GLOBAL_CHAT);
		}
	}
	
	void SendSimpleChatMessage(PlayerIdentity player, string message, bool global = false) {
		if (!player && !global)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(CCSystem);
		rpc.Write("");
		rpc.Write("+" + message);
		rpc.Write("");
		rpc.Write("");
		rpc.Write(0);
		rpc.Write("");
		rpc.Write(0);
		rpc.Send(NULL, OBLPartyRPCs.OBL_GLOBAL_CHAT, true, player);
	}
	
	bool ChatCommandExists(string cmd) {
		return (cmd == "mute" || cmd == "muteid" || cmd == "unmute" || cmd == "unmuteid" || cmd == "mutereloadconfig" || cmd == "mutechannel" || cmd == "unmutechannel" || cmd == "votemute" || cmd == "mutevote");
	}
	
	bool HasPermission(PlayerIdentity sender, string cmd) {
		if (ChatCommandExists(cmd)) {
			if (cmd == "votemute" || cmd == "mutevote")
				return true;
			MuteConfig cfg = GetMuteConfig();
			string steamid = sender.GetPlainId();
			return GetMuteConfig().IsAdmin(steamid);
		}
		return false;
	}
	
	void OnChatCommand(PlayerIdentity sender, string cmd, TStringArray args) {
		if (!ChatCommandExists(cmd))
			return;
		GetGame().AdminLog("On Chat Command: " + sender.GetPlainId() + " Cmd: " + cmd + " ArgsCount: " + args.Count());
		string printcmd = cmd + "";
		printcmd.Replace("%", "");
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("On Chat Command: " + sender.GetPlainId() + " Cmd: " + printcmd + " ArgsCount: " + args.Count());
		if (!HasPermission(sender, cmd)) {
			SendSimpleChatMessage(sender, "У вас немає прав для цієї команди!");
			return;
		}
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("CMD: " + printcmd);
		foreach (string arg : args) {
			if (OBLLogger.IsDebug())
				OBLLogger.Debug("Arg: " + arg);
		}
		if (cmd == "mute") {
			if (args.Count() < 2) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !mute [Ім'яГравця] [Час у хвилинах]");
				return;
			}
			string name = args.Get(0);
			int duration = args.Get(1).ToInt();
			
			PlayerIdentity target = GetPlayerIdentityByName(name);
			if (!target) {
				SendSimpleChatMessage(sender, "Не вдалося знайти гравця " + name + "! Спробуйте !muteid [Steamid або BIUID] [Час у хвилинах]");
				return;
			}
			MuteChatPlayer(target, duration);
			SendSimpleChatMessage(sender, "Гравця вимкнено в чаті на " + duration + " хвилин");
		} else if (cmd == "muteid") {
			if (args.Count() < 2) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !muteid [Steamid або BIUID] [Час у хвилинах]");
				return;
			}
			string steamid = args[0];
			duration = args[1].ToInt();
			
			target = GetPlayerIdentityById(steamid);
			if (!target) {
				SendSimpleChatMessage(sender, "Не вдалося знайти гравця " + steamid + "! Спробуйте !muteid [Steamid або BIUID] [Час у хвилинах]");
				return;
			}
			MuteChatPlayer(target, duration);
			SendSimpleChatMessage(sender, "Гравця вимкнено в чаті на " + duration + " хвилин");
		} else if (cmd == "unmute") {
			if (args.Count() < 1) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !unmute [Ім'яГравця]");
				return;
			}
			name = args[0];
			
			target = GetPlayerIdentityByName(name);
			if (!target) {
				string printname = name + "";
				printname.Replace("%", "");
				SendSimpleChatMessage(sender, "Не вдалося знайти гравця " + printname + "! Спробуйте !unmuteid [Steamid або BIUID]");
				return;
			}
			UnMuteChatPlayer(target);
			SendSimpleChatMessage(sender, "Гравця знову ввімкнено в чаті");
		} else if (cmd == "unmuteid") {
			if (args.Count() < 1) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !unmuteid [Steamid або BIUID]");
				return;
			}
			steamid = args[0];
			
			target = GetPlayerIdentityById(steamid);
			if (!target) {
				SendSimpleChatMessage(sender, "Не вдалося знайти гравця " + steamid + "! Спробуйте !unmuteid [Steamid або BIUID]");
				return;
			}
			UnMuteChatPlayer(target);
			SendSimpleChatMessage(sender, "Гравця знову ввімкнено в чаті");
		} else if (cmd == "mutereloadconfig") {
			ReloadMuteConfig();
			SendSimpleChatMessage(sender, "Конфіг перезавантажено");
		} else if (cmd == "mutechannel") {
			if (args.Count() < 1) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !mutechannel [НазваКаналу]");
				return;
			}
			string chatName = args[0];
			ChannelCfg channel = GetMuteConfig().FindChannelByName(chatName);
			if (!channel) {
				SendSimpleChatMessage(sender, "Канал \"" + chatName + "\" не знайдено!");
				return;
			}
			if (channel.muted) {
				SendSimpleChatMessage(sender, "Канал \"" + chatName + "\" вже вимкнено!");
				return;
			}
			channel.muted = true;
			SendSimpleChatMessage(null, "Канал \"" + chatName + "\" вимкнув " + sender.GetName() + "!", true);
			return;
		} else if (cmd == "unmutechannel") {
			if (args.Count() < 1) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !unmutechannel [НазваКаналу]");
				return;
			}
			chatName = args[0];
			channel = GetMuteConfig().FindChannelByName(chatName);
			if (!channel) {
				SendSimpleChatMessage(sender, "Канал \"" + chatName + "\" не знайдено!");
				return;
			}
			if (!channel.muted) {
				SendSimpleChatMessage(sender, "Канал \"" + chatName + "\" не був вимкнений!");
				return;
			}
			channel.muted = false;
			SendSimpleChatMessage(null, "Канал \"" + chatName + "\" увімкнув " + sender.GetName() + "!", true);
			return;
		} else if (cmd == "votemute" || cmd == "mutevote") {
			// OBL FIX: enableMuteVote from ChatConfig.json was ignored
			if (!GetMuteConfig().enableMuteVote) {
				SendSimpleChatMessage(sender, "Голосування за мут вимкнено на цьому сервері.");
				return;
			}
			int onlinePlayers = GetOnlinePlayerCount();
			if (onlinePlayers < GetMuteConfig().muteVoteMinPlayers) {
				SendSimpleChatMessage(sender, "Для голосування за мут потрібно щонайменше " + GetMuteConfig().muteVoteMinPlayers + " гравців онлайн");
				return;
			}
			if (args.Count() < 1) {
				SendSimpleChatMessage(sender, "Недостатньо аргументів! !votemute [Ім'яГравця]");
				return;
			}
			name = args[0];
			
			target = GetPlayerIdentityByName(name);
			if (!target) {
				printname = name + "";
				printname.Replace("%", "");
				SendSimpleChatMessage(sender, "Не вдалося знайти гравця " + printname);
				return;
			}
			if (target.GetPlainId() == sender.GetPlainId()) {
				SendSimpleChatMessage(sender, "Не можна голосувати за мут самого себе!");
				return;
			}
			if (GetMuteConfig().IsAdmin(target.GetPlainId())) {
				SendSimpleChatMessage(sender, "Не можна голосувати за мут адміністратора!");
				return;
			}
			if (GetMuteConfig().IsMuted(target.GetPlainId())) {
				SendSimpleChatMessage(sender, "Цей гравець уже вимкнений у чаті!");
				return;
			}
			TStringSet muteVoter = new TStringSet;
			steamid = target.GetPlainId();
			if (muteVotes.Contains(steamid))
				muteVoter = muteVotes.Get(steamid);
			else
				muteVotes.Insert(steamid, muteVoter);
			int count = muteVoter.Count();
			muteVoter.Insert(sender.GetPlainId());
			if (muteVoter.Count() == count) {
				SendSimpleChatMessage(sender, "Ви вже голосували за цього гравця!");
				return;
			}
			count = muteVoter.Count();
			// OBL FIX: round up and never allow a single vote to mute
			int minPlayers = Math.Ceil(onlinePlayers * GetMuteConfig().muteVotePercentile);
			if (minPlayers < 2)
				minPlayers = 2;

			if (count >= minPlayers) {
				MuteChatPlayer(target, GetMuteConfig().muteVoteMuteTimeMins);
				SendSimpleChatMessage(null, "" + target.GetName() + " вимкнено в чаті на " + GetMuteConfig().muteVoteMuteTimeMins + " хв", true);
				muteVoter.Clear();
				return;
			}
			SendSimpleChatMessage(null, "" + sender.GetName() + " почав голосування за мут " + target.GetName() + ". Напишіть !votemute " + target.GetName() + ", якщо погоджуєтесь. (" + count + "/" + minPlayers + " голосів)", true);
		}
	}
	
	void UnMuteChatPlayer(PlayerIdentity ident) {
		GetGame().AdminLog("Player Was Unmuted: " + ident.GetPlainId() + " (" + ident.GetName() + ")");
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Player Was Unmuted: " + ident.GetPlainId() + " (" + ident.GetName() + ")");
		GetMuteConfig().UnMutePlayer(ident.GetPlainId());
		GetMuteConfig().SaveConfig();
	}
	
	void MuteChatPlayer(PlayerIdentity ident, int minutes) {
		GetGame().AdminLog("Player Was Muted: " + ident.GetPlainId() + " (" + ident.GetName() + ") for " + minutes + " Minutes");
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Player Was Muted: " + ident.GetPlainId() + " (" + ident.GetName() + ") for " + minutes + " Minutes");
		GetMuteConfig().MutePlayer(ident.GetPlainId(), minutes);
		GetMuteConfig().SaveConfig();
	}
	
	PlayerIdentity GetPlayerIdentityById(string id) {
		bool steamid = id.Length() == 17;
		array<PlayerIdentity> identities = new array<PlayerIdentity>();
		GetGame().GetPlayerIndentities(identities);
		
		foreach (PlayerIdentity ident : identities) {
			if (!ident)
				continue;
			if (steamid) {
				if (ident.GetPlainId() == id)
					return ident;
			} else {
				if (ident.GetId() == id)
					return ident;
			}
		}
		return null;
	}
	
	PlayerIdentity GetPlayerIdentityByName(string name) {
		array<PlayerIdentity> identities = new array<PlayerIdentity>();
		GetGame().GetPlayerIndentities(identities);
		string nameLower = name;
		nameLower.ToLower();
		string nameUnderscore = nameLower;
		nameUnderscore.Replace(" ", "_");
		nameUnderscore.Replace("	", "_");
		foreach (PlayerIdentity ident : identities) {
			if (!ident)
				continue;
			string identName = ident.GetName();
			if (identName == name)
				return ident;
			identName.ToLower();
			if (identName == nameLower)
				return ident;
			identName.Replace(" ", "_");
			identName.Replace("	", "_");
			if (identName == nameUnderscore)
				return ident;
		}
		return null;
	}
	#endif
}
