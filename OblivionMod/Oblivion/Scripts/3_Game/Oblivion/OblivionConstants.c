const string OBLIVION_MOD_VERSION   = "0.10.0";
const string OBLIVION_PROFILE_DIR   = "$profile:Oblivion/";
const string OBLIVION_SETTINGS_FILE  = "$profile:Oblivion/settings.json";
const string OBLIVION_CONTRACTS_FILE = "$profile:Oblivion/contracts.json";

// Сервер -> клієнт: синхронізація налаштувань при підключенні.
const int OBLIVION_RPC_SETTINGS = 0x0B1100;

// Клієнт -> сервер: команда з чату (/contract ...).
const int OBLIVION_RPC_COMMAND = 0x0B1101;

// Розбиває рядок чату на слова; перше слово — команда.
static void OblivionSplitCommand(string text, out array<string> words)
{
	words = new array<string>();
	array<string> raw = new array<string>();
	text.Split(" ", raw);
	foreach (string w : raw)
	{
		if (w != "")
			words.Insert(w);
	}
}

static bool OblivionIsChatCommand(string text)
{
	array<string> words;
	OblivionSplitCommand(text, words);
	if (words.Count() == 0)
		return false;

	string cmd = words[0];
	cmd.ToLower();
	return cmd == "/contract" || cmd == "/contracts" || cmd == "/контракт" || cmd == "/контракти"
		|| cmd == "/bounty" || cmd == "/баунті";
}
