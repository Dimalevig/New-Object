// Палітра Oblivion (з прапора сервера). Кольори розмітки (.layout) задані тими ж значеннями.
class OBLTheme {
	// версія палітри: при зміні гравцям один раз скидаються кольори маркерів/HUD на нові
	static const int PALETTE_VERSION = 1;

	static int Background()  { return ARGB(225, 13, 11, 26); }    // #0D0B1A
	static int Panel()       { return ARGB(220, 23, 20, 43); }    // #17142B
	static int Accent()      { return ARGB(255, 123, 92, 255); }  // #7B5CFF
	static int AccentLight() { return ARGB(255, 185, 168, 255); } // #B9A8FF
	static int Glow()        { return ARGB(255, 199, 125, 255); } // #C77DFF
	static int Text()        { return ARGB(255, 228, 224, 242); } // #E4E0F2
	static int Muted()       { return ARGB(255, 142, 136, 172); } // #8E88AC
	static int Success()     { return ARGB(255, 91, 227, 166); }  // #5BE3A6
	static int Danger()      { return ARGB(255, 224, 70, 90); }   // #E0465A
}
