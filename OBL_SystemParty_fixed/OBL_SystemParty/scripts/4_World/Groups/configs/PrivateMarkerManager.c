class OBLPrivateMarkerManager {

	ref array<ref Param2<string, ref array<ref OBLMarker>>> markers = new array<ref Param2<string, ref array<ref OBLMarker>>>();
	ref array<ref OBLMarker> privateMarkers = new array<ref OBLMarker>();
	string currentServer;
	
	static ref OBLPrivateMarkerManager g_OBLPrivateMarkerManager;
	
	static void Delete() {
		if (g_OBLPrivateMarkerManager)
			delete g_OBLPrivateMarkerManager;
	}
	
	void ~OBLPrivateMarkerManager() {
		Save();
		foreach (OBLMarker marker : privateMarkers)
			delete marker;
		privateMarkers.Clear();
	}
	
	static OBLPrivateMarkerManager Get(string server = "") {
		if (!g_OBLPrivateMarkerManager) {
			g_OBLPrivateMarkerManager = Load(server);
			g_OBLPrivateMarkerManager.InitMarkers();
		}
		return g_OBLPrivateMarkerManager;
	}
	
	static OBLPrivateMarkerManager Load(string server) {
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Loading Private Markers for Server " + server);
		OBLPrivateMarkerManager mgr;
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER)) {
			mgr = LoadDefault();
			mgr.currentServer = server;
			JsonFileLoader<ref array<ref Param2<string, ref array<ref OBLMarker>>>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER, mgr.markers);
			return mgr;
		}
		mgr = new OBLPrivateMarkerManager();
		mgr.currentServer = server;
		OBLLogger.Debug("JsonLoadFile OBLPrivateMarkerManager.json");
		JsonFileLoader<ref array<ref Param2<string, ref array<ref OBLMarker>>>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER, mgr.markers);
		return mgr;
	}
	
	static OBLPrivateMarkerManager LoadDefault() {
		OBLPrivateMarkerManager def = new OBLPrivateMarkerManager;
		return def;
	}
	
	void Save() {
		JsonFileLoader<ref array<ref Param2<string, ref array<ref OBLMarker>>>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_PRIVATE_MARKER, markers);
	}
	
	void InitMarkers() {
		privateMarkers = GetPrivateMarkersFromList();
		foreach (OBLMarker mark : privateMarkers) {
			if (!mark)
				continue;
			mark.InitMarker();
			// OBL FIX: the constructor already registers the marker; don't add it twice
			if (OBLMarker.allMarkers.Find(mark) == -1)
				mark.AddToAllList();
		}
	}
	
	private array<ref OBLMarker> GetPrivateMarkersFromList() {
		foreach (Param2<string, ref array<ref OBLMarker>> param : markers) {
			if (param.param1 == currentServer) {
				return param.param2;
			}
		}
		privateMarkers = new array<ref OBLMarker>();
		markers.Insert(new Param2<string, ref array<ref OBLMarker>>(currentServer, privateMarkers));
		return privateMarkers;
	}
	
	void AddMarker(OBLMarker marker) {
		privateMarkers.Insert(marker);
		marker.InitMarker();
		Save();
	}

	// прибирає попередні мітки з тією ж назвою та іконкою (напр. «Місце смерті»)
	void RemoveMarkersLike(string name, string icon) {
		for (int i = privateMarkers.Count() - 1; i >= 0; i--) {
			OBLMarker old = privateMarkers.Get(i);
			if (old && old.name == name && old.icon == icon) {
				privateMarkers.Remove(i);
				delete old;
			}
		}
	}
	
	void RemoveMarker(OBLMarker marker) {
		privateMarkers.RemoveItem(marker);
		delete marker;
		Save();
	}
	
	OBLMarker FindMarkerByUID(int uid) {
		foreach (OBLMarker marker : privateMarkers) {
			if (marker && marker.uid == uid)
				return marker;
		}
		return null;
	}
	
	bool FindMarkerByIcon(string icon, out OBLMarker mark) {
		foreach (OBLMarker marker : privateMarkers) {
			if (marker && marker.icon == icon) {
				mark = marker;
				return true;
			}
		}
		return false;
	}
	
	bool FindNearestMarker(vector position, out OBLMarker markero, out float distance) {
		if (privateMarkers.Count() == 0)
			return false;
		float bestDist = 0;
		OBLMarker bestMarker = null;
		foreach (OBLMarker marker : privateMarkers) {
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