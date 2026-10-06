modded class OBLStaticMarkerManager {
	
	override void RPC_OBL(PlayerIdentity sender, ParamsReadContext ctx, int type) {
		if (type == OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS) {
			ScriptRPC rpc = new ScriptRPC();
			int markerCount = 0;
			foreach (OBLServerMarker markerCountStatic : staticMarkers) {
				if (markerCountStatic)
					markerCount++;
			}
			foreach (OBLServerMarker markerCountTemp : tempStaticMarkers) {
				if (markerCountTemp)
					markerCount++;
			}
			rpc.Write(markerCount);
			foreach (OBLServerMarker marker : staticMarkers) {
				if (marker)
					marker.WriteToCtx(rpc);
			}
			foreach (OBLServerMarker marker2 : tempStaticMarkers) {
				if (marker2)
					marker2.WriteToCtx(rpc);
			}
			rpc.Send(null, OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS, true, sender);
		} else if (type == OBLPartyRPCs.CONFIG_GLOBAL_MARKER_ADD) {
			if (!sender)
				return;
			string steamid = sender.GetPlainId();
			if (OBLPartyMainConfig.Get().adminSteamids.Find(steamid) == -1) {
				OBLLogger.Debug("Player " + steamid + " tried to add Global Marker without Permission !");
				return;
			}
			OBLMarker mark = new OBLMarker();
			if (!mark.ReadFromCtx(ctx)) {
				OBLLogger.Debug("Failed to read Global Marker from " + steamid);
				return;
			}
			OBLServerMarker serverMark = new OBLServerMarker();
			serverMark.InitFromOBLMarker(mark);
			if (serverMark.type == OBLMarkerType.SERVER_DYNAMIC) {
				tempStaticMarkers.Insert(serverMark);
			} else {
				serverMark.type = OBLMarkerType.SERVER_STATIC;
				staticMarkers.Insert(serverMark);
				SaveMarkers();
			}
			OBLPartyManager.Get().SendInfoNotification(sender, "Маркер додано!");
			GetGame().AdminLog("Global Marker " + serverMark.name + " (" + serverMark.uid + ") was added by " + steamid + " " + sender.GetName());
			SendMarkerRefreshRPC();
		} else if (type == OBLPartyRPCs.CONFIG_GLOBAL_MARKER_REMOVE) {
			if (!sender)
				return;
			steamid = sender.GetPlainId();
			if (OBLPartyMainConfig.Get().adminSteamids.Find(steamid) == -1) {
				OBLLogger.Debug("Player " + steamid + " tried to remove Global Marker without Permission !");
				return;
			}
			int uid = 0;
			if (!ctx.Read(uid)) {
				OBLLogger.Debug("Failed to read UID for Global Marker Remove from " + steamid);
				return;
			}
			OBLServerMarker marker3 = FindMarker(uid, staticMarkers);
			if (!marker3)
				marker3 = FindMarker(uid, tempStaticMarkers);
			else
				staticMarkers.RemoveItem(marker3);
			if (!marker3) {
				OBLPartyManager.Get().SendInfoNotification(sender, "Маркер не знайдено!");
				return;
			} else
				tempStaticMarkers.RemoveItem(marker3);
			OBLPartyManager.Get().SendInfoNotification(sender, "Маркер видалено!");
			GetGame().AdminLog("Global Marker " + marker3.name + " (" + marker3.uid + ") was removed by " + steamid + " " + sender.GetName());
			SendMarkerRefreshRPC();
			SaveMarkers();
		}
	}
	
	void SaveMarkers() {
		JsonFileLoader<array<ref OBLServerMarker>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_STATIC_MARKER, staticMarkers);
	}
	
	OBLServerMarker FindMarker(int uid, array<ref OBLServerMarker> lst) {
		foreach (OBLServerMarker marker : lst) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}
}
