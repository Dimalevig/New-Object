# Oblivion — DayZ мод для сервера Oblivion

Серверний мод зі спеціальними механіками. Список механік — у [MECHANICS.md](MECHANICS.md).

## Структура

```
OblivionMod/
├── MECHANICS.md                 # опис усіх механік
└── Oblivion/                    # пакується в Oblivion.pbo
    ├── config.cpp               # CfgPatches / CfgMods
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
