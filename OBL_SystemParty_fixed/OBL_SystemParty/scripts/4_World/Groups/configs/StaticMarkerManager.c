class OBLStaticMarkerManager {

	ref array<ref OBLServerMarker> staticMarkers = new array<ref OBLServerMarker>();
	ref array<ref OBLServerMarker> tempStaticMarkers = new array<ref OBLServerMarker>();
	
	static ref OBLStaticMarkerManager g_OBLStaticMarkerManager;
	static ref array<ref OBLServerMarker> copyStaticMarkers = new array<ref OBLServerMarker>();
	
	static OBLStaticMarkerManager Get() {
		if (!g_OBLStaticMarkerManager) {
			g_OBLStaticMarkerManager = Load();
			RemoveItem(g_OBLStaticMarkerManager);
		}
		return g_OBLStaticMarkerManager;
	}
	
	static void Delete() {
		if (g_OBLStaticMarkerManager)
			delete g_OBLStaticMarkerManager;
	}
	
	static OBLStaticMarkerManager Load() {
		OBLStaticMarkerManager markerMgr;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_STATIC_MARKER)) {
			markerMgr = LoadDefault();
			JsonFileLoader<array<ref OBLServerMarker>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_STATIC_MARKER, markerMgr.staticMarkers);
			return markerMgr;
		}
		markerMgr = new OBLStaticMarkerManager();
		OBLLogger.Debug("JsonLoadFile OBLStaticMarkerManager.json");
		JsonFileLoader<array<ref OBLServerMarker>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_STATIC_MARKER, markerMgr.staticMarkers);
		return markerMgr;
	}

	static void RemoveItem(OBLStaticMarkerManager mkManager) {
		if (!mkManager || !mkManager.staticMarkers)
			return;
		Print("[SystemParty] - RemoveItem");
		for (int i = 0; i < mkManager.staticMarkers.Count(); i++) {
			OBLServerMarker marker = mkManager.staticMarkers.Get(i);
			if (marker != null && ( marker.temp == true || marker.temp == 1 )) {
				mkManager.staticMarkers.Remove(i);
				i--;
			}
		}
		JsonFileLoader<array<ref OBLServerMarker>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_STATIC_MARKER, mkManager.staticMarkers);
	}
	
	static OBLStaticMarkerManager LoadDefault() {
		OBLStaticMarkerManager mgr = new OBLStaticMarkerManager();
		OBLServerMarker marker = new OBLServerMarker();
		marker.Init("Токсична зона", "1590 0 14123", "DZ\\gear\\navigation\\data\\map_tree_ca.paa", 255, 0, 0);
		mgr.staticMarkers.Insert(marker);
		return mgr;
	}
	
	void SendMarkerRefreshRPC() {
		RPC_OBL(null, null, OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS);
	}
	
	void RPC_OBL(PlayerIdentity sender, ParamsReadContext ctx, int type) {}

	OBLServerMarker AddTempServerMarker(string name_, vector position_, string icon_, int color_, bool toSurface = true, bool display3d = true, bool displayMap = true, bool displayGPS = true) {
		OBLServerMarker marker = new OBLServerMarker();

		int r_ = (color_ >> 16) & 0xFF;
		int g_ = (color_ >> 8) & 0xFF;
		int b_ = color_ & 0xFF;

		marker.Init(name_, position_, icon_, r_, g_, b_, toSurface, display3d, displayMap);
		marker.SetVisibleOnScreen(true);
		staticMarkers.Insert(marker);
		SendMarkerRefreshRPC();
		return marker;
	}

	OBLServerMarker FindTempMarker(int uid) {
		foreach (OBLServerMarker marker : staticMarkers) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}

	void RemoveServerMarker(OBLServerMarker marker) {
		if (!marker)
			return;
		marker.SetVisibleOnScreen(false);
		staticMarkers.RemoveItem(marker);
		SendMarkerRefreshRPC();
	}

}

class OBLStaticMarkerManagerClient {

	ref array<ref OBLServerMarker> staticMarkers = new array<ref OBLServerMarker>();
	
	static ref OBLStaticMarkerManagerClient g_OBLPartyGroups;
	
	static void Delete() {
		if (g_OBLPartyGroups)
			delete g_OBLPartyGroups;
	}
	
	void ~OBLStaticMarkerManagerClient() {
		foreach (OBLServerMarker marker : staticMarkers)
			delete marker;
		staticMarkers.Clear();
	}
	
	static OBLStaticMarkerManagerClient Get() {
		if (!g_OBLPartyGroups) {
			g_OBLPartyGroups = new OBLStaticMarkerManagerClient();
			GetGame().RPCSingleParam(null, OBLPartyRPCs.CONFIG_SYNC_STATIC_MARKERS, new Param1<bool>(true), true);
		}
		return g_OBLPartyGroups;
	}
	
	void RequestGlobalMarkerAdd(OBLMarker marker) {
		ScriptRPC rpc = new ScriptRPC();
		marker.WriteToCtx(rpc);
		rpc.Send(null, OBLPartyRPCs.CONFIG_GLOBAL_MARKER_ADD, true);
	}

	void RequestGlobalMarkerUpdate(OBLMarker marker) {
		RequestGlobalMarkerRemove(marker.uid);
		RequestGlobalMarkerAdd(marker);
	}
	
	void RequestGlobalMarkerRemove(int uid) {
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(uid);
		rpc.Send(null, OBLPartyRPCs.CONFIG_GLOBAL_MARKER_REMOVE, true);
	}
	
	void RPC_OBL(PlayerIdentity sender, ParamsReadContext ctx) {
		int count;
		if (!ctx.Read(count)) {
			OBLLogger.Warn("Unable to receive Static Markers from Server !");
			return;
		}
		DeleteStaticMarkers();
		for (int i = 0; i < count; i++) {
			OBLServerMarker mark = new OBLServerMarker;
			if (mark.ReadFromCtx(ctx)) {
				staticMarkers.Insert(mark);
			} else {
				OBLLogger.Warn("Failed to read received Markers from Server. Index: " + i);
				return;
			}
		}
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Received Static Markers from Server: " + staticMarkers.Count());
		InitAllMarkers();
	}
	
	void InitAllMarkers() {
		foreach (OBLServerMarker marker : staticMarkers) {
			marker.InitMarker();
		}
	}
	
	void DeleteStaticMarkers() {
		foreach (OBLServerMarker marker : staticMarkers) {
			delete marker;
		}
		staticMarkers.Clear();
	}
	
	void AddMarker(OBLServerMarker marker) {
		staticMarkers.Insert(marker);
	}
	
	void RemoveMarker(OBLServerMarker marker) {
		staticMarkers.RemoveItem(marker);
	}
	
	OBLServerMarker FindMarkerByUID(int uid) {
		foreach (OBLServerMarker marker : staticMarkers) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}
	
	bool FindNearestMarker(vector position, out OBLMarker markero, out float distance) {
		if (staticMarkers.Count() == 0)
			return false;
		float bestDist = 0;
		OBLMarker bestMarker = null;
		foreach (OBLMarker marker : staticMarkers) {
			if (!marker)
				continue;
			float markerDist = vector.Distance(position, marker.position);
			// OBL FIX: bestDist was assigned a bool (the comparison) instead of the distance
			if (!bestMarker || markerDist < bestDist) {
				bestMarker = marker;
				bestDist = markerDist;
			}
		}
		if (bestMarker) {
			markero = bestMarker;
			distance = bestDist;
			return true;
		}
		return false;
	}
	
}
