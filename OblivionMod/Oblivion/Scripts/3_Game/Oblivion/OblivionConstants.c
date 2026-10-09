const string OBLIVION_MOD_VERSION   = "0.11.0";
const string OBLIVION_PROFILE_DIR   = "$profile:Oblivion/";
const string OBLIVION_SETTINGS_FILE  = "$profile:Oblivion/settings.json";
const string OBLIVION_CONTRACTS_FILE = "$profile:Oblivion/contracts.json";
const string OBLIVION_PLAYTIME_FILE  = "$profile:Oblivion/playtime.json";

// Сервер -> клієнт: синхронізація налаштувань при підключенні.
const int OBLIVION_RPC_SETTINGS = 0x0B1100;

// Клієнт -> сервер: команда з чату (!contract ... або /contract ...).
const int OBLIVION_RPC_COMMAND = 0x0B1101;

// Клієнт -> сервер: швидкий слот (1-9) у машині — річ у руки кладе сервер.
const int OBLIVION_RPC_QUICKBAR = 0x0B1102;

// Розбиває рядок чату на слова; перше слово — команда.
void OblivionSplitCommand(string text, out array<string> words)
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

// "!reward" і "/reward" — одне й те саме; повертає "/reward" у нижньому регістрі.
string OblivionNormalizeCommand(string word)
{
	string cmd = word;
	cmd.ToLower();
	if (cmd.Length() > 1 && cmd.Substring(0, 1) == "!")
		cmd = "/" + cmd.Substring(1, cmd.Length() - 1);
	return cmd;
}

bool OblivionIsChatCommand(string text)
{
	array<string> words;
	OblivionSplitCommand(text, words);
	if (words.Count() == 0)
		return false;

	string cmd = OblivionNormalizeCommand(words[0]);
	array<string> commands = {"/contract", "/contracts", "/контракт", "/контракти", "/bounty", "/баунті", "/reward", "/нагорода"};
	return commands.Find(cmd) != -1;
}
