# Oblivion — DayZ мод для сервера Oblivion

Серверний мод зі спеціальними механіками. Список механік — у [MECHANICS.md](MECHANICS.md).

## Структура

```
OblivionMod/
├── MECHANICS.md                 # опис усіх механік
└── Oblivion/                    # пакується в Oblivion.pbo
    ├── config.cpp               # CfgPatches / CfgMods
    ├── stringtable.csv          # тексти в грі (українською в усіх колонках)
    └── Scripts/
        ├── 3_Game/Oblivion/     # константи, налаштування
        ├── 4_World/Oblivion/    # гравці, предмети, дії
        └── 5_Mission/Oblivion/  # серверна/клієнтська місія
```

## Збірка

1. Скопіюйте папку `Oblivion` на диск `P:\` (DayZ Tools → Workdrive).
2. Запакуйте через **Addon Builder** (DayZ Tools) з префіксом `Oblivion`.
3. Покладіть результат у `@Oblivion/Addons/Oblivion.pbo` (+ підпишіть ключем, `.bikey` → `keys/` сервера).
4. Запускайте сервер з `-mod=@Oblivion`. Мод **клієнт-серверний** (нові дії в грі), тому гравці теж мають його завантажити.

## Налаштування

При першому запуску сервер створює `<profiles>/Oblivion/settings.json` з дефолтними значеннями.

## Інтеграція з модом пати (OBL_SystemPartyServer)

Мод пати замінює ванільний чат, тому команди Oblivion (`!reward`, `!bounty`, `!contract`, `!contracts`) треба передати з нього. У `OBL_SystemPartyServer/scripts/5_mission/groups-server/mission/missionserver.c` у `OnChatCommand` невідомі команди передаються в Oblivion **викликом за назвою функції** (`CallFunctionParams(..., "OblivionRunChatCommand", ...)`):

```c
	void OnChatCommand(PlayerIdentity sender, string cmd, TStringArray args) {
		if (!ChatCommandExists(cmd)) {
			PlayerBase oblPlayer = PlayerBase.GetPlayerByIdentity(sender);
			if (oblPlayer) {
				string oblText = "/" + cmd;
				foreach (string oblArg : args)
					oblText += " " + oblArg;
				GetGame().GameScript.CallFunctionParams(oblPlayer, "OblivionRunChatCommand", null, new Param1<string>(oblText));
			}
			return;
		}
		...
```

Відповідь Oblivion так само викликає `SendSimpleChatMessage` мода пати за назвою. Жорсткої залежності немає: кожен мод працює і без іншого.
