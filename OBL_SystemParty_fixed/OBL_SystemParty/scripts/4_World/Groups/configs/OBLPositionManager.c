typedef Param3<string, ref vector, int> OBLWidgetPosition;

class OBLPositionManager {

	static ref OBLPositionManager g_OBLPositionManager;
	
	ref array<ref OBLWidgetPosition> positions = new array<ref OBLWidgetPosition>();
	string changedPosition = "";
	
	static ref ScriptInvoker Event_OnPositionChange = new ScriptInvoker();
	
	static OBLPositionManager Get() {
		if (!g_OBLPositionManager) {
			g_OBLPositionManager = Load();
		}
		return g_OBLPositionManager;
	}
	
	static OBLPositionManager Load() {
		if (!FileExist(OBLPartyConstants.SAVE_PREFIX))
			MakeDirectory(OBLPartyConstants.SAVE_PREFIX);
		OBLPositionManager mgr = new OBLPositionManager();
		ref array<ref OBLWidgetPosition> positionss = new array<ref OBLWidgetPosition>();
		if (FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_POSITION_MANAGER)) {
			OBLLogger.Debug("JsonLoadFile OBLPositionManager.json");
			JsonFileLoader<array<ref OBLWidgetPosition>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_POSITION_MANAGER, positionss);
		}
		if (OBLLogger.IsDebug())
			OBLLogger.Debug("Loaded Positions: " + positionss.Count());
		mgr.SetDefaultPositions();
		mgr.ReplacePositions(positionss);
		mgr.Save();
		return mgr;
	}
	
	static void Reload() {
		g_OBLPositionManager = Load();
		InvokeOnChanged();
	}
	
	void Save() {
		ref array<ref OBLWidgetPosition> positionssave = new array<ref OBLWidgetPosition>();
		ref array<ref OBLWidgetPosition> positionss = new array<ref OBLWidgetPosition>();
		if (FileExist(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_POSITION_MANAGER)) {
			JsonFileLoader<array<ref OBLWidgetPosition>>.JsonLoadFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_POSITION_MANAGER, positionss);
		}
		// Add all Current Position to the Array
		foreach (OBLWidgetPosition pos : positions) {
			if (pos)
				positionssave.Insert(pos);
		}
		// Go through all Positions that were saved in the config to not delete entries from other servers.
		foreach (OBLWidgetPosition oldPos : positionss) {
			bool found = false;
			// Check if the Position is already present and ignore them
			foreach (OBLWidgetPosition setPos : positionssave) {
				if (oldPos.param1 == setPos.param1) {
					found = true;
					break;
				}
			}
			// if pos was not found, add it to the list
			if (!found && oldPos)
				positionssave.Insert(oldPos);
		}
		JsonFileLoader<array<ref OBLWidgetPosition>>.JsonSaveFile(OBLPartyConstants.SAVE_PREFIX + OBLPartyConstants.SAVE_SUFFIX_POSITION_MANAGER, positionssave);
	}
	
	private void ReplacePositions(array<ref OBLWidgetPosition> positionss) {
		foreach (OBLWidgetPosition pos : positionss) {
			SetPosition(pos.param1, pos.param2, false);
			SetIndex(pos.param1, pos.param3, false);
		}
	}
	
	void ResetAll() {
		positions.Clear();
		SetDefaultPositions();
		InvokeOnChanged();
	}
	
	static void InvokeOnChanged() {
		Event_OnPositionChange.Invoke();
	}
	
	vector GetPosition(string posStr, vector defaultPos = vector.Zero, int defaultIndex = 0, bool insert = true) {
		if (!insert)
			return vector.Zero;
		if (changedPosition == posStr) {
			SetPosition(posStr, defaultPos);
			SetIndex(posStr, defaultIndex);
			changedPosition = "";
		}
		foreach (OBLWidgetPosition pos : positions) {
			if (pos && pos.param1 == posStr)
				return pos.param2;
		}
		positions.Insert(new OBLWidgetPosition(posStr, defaultPos, defaultIndex));
		return defaultPos;
	}
	
	int GetIndex(string posStr) {
		foreach (OBLWidgetPosition pos : positions) {
			if (pos && pos.param1 == posStr)
				return pos.param3;
		}
		return 0;
	}
	
	void SetPosition(string posStr, vector pos, bool add = true) {
		foreach (OBLWidgetPosition poss : positions) {
			if (poss && poss.param1 == posStr) {
				poss.param2 = pos;
				return;
			}
		}
		if (add)
			positions.Insert(new OBLWidgetPosition(posStr, pos, 0));
	}
	
	void SetIndex(string posStr, int index, bool add = true) {
		foreach (OBLWidgetPosition poss : positions) {
			if (poss && poss.param1 == posStr) {
				poss.param3 = index;
				return;
			}
		}
		if (add)
			positions.Insert(new OBLWidgetPosition(posStr, vector.Zero, index));
	}
	
	void SetDefaultPositions() {
		GetPosition("PlayerList", Vector(0.02, 0.02, 0), 0, OBLPartyMainConfig.Get().enablePlayerList);
		#ifndef OBL_DISABLE_CHAT
		GetPosition("Chat", Vector(0.05, 0.85, 0), 6);
		#endif
	}
	
	void ResetPositionToDefault(string posStr) {
		changedPosition = posStr;
		SetDefaultPositions();
	}
	
	TStringArray GetPositionStrings() {
		ref TStringArray arr = new TStringArray();
		foreach (OBLWidgetPosition pos : positions) {
			arr.Insert(pos.param1);
		}
		return arr;
	}

}