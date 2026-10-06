modded class IngameHud {

	bool OBLIsHudVisible() {
		#ifdef DAYZ_1_26
				return IsHudVisible();
		#else
				return !GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS);
		#endif
	}

}