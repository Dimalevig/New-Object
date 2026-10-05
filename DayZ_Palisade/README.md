# Частокіл (Palisade) для DayZ — ванільний стиль

Будівельний об'єкт «частокіл», зроблений так само, як ванільний паркан (`Fence`) / `FenceKit`:
кіт → голограма → розміщення → поетапне будівництво з матеріалів інструментами → розбирання → кіт назад.

## Структура

```
PalisadeMod/
├── config.cpp                       # CfgPatches, CfgMods, CfgVehicles (кіт, голограма, частокіл)
├── stringtable.csv                  # назви/описи
├── data/
│   └── model.cfg                    # шаблон анімацій (hide) для palisade.p3d
└── scripts/4_World/PalisadeMod/
    ├── entities/PalisadeKit.c       # кіт (extends KitBase)
    ├── entities/Palisade.c          # частокіл (extends BaseBuildingBase)
    ├── recipes/CraftPalisadeKit.c   # рецепт крафту кіта
    └── plugins/PluginRecipesManager.c  # реєстрація рецепта
server/
└── types.xml                        # записи для серверної економіки
```

## Геймплей

| Крок | Що потрібно | Інструмент |
|------|-------------|------------|
| Крафт кіта | 4× `WoodenStick` + `Rope` | — |
| Розміщення | кіт у руках → «Розмістити» | — |
| `base` (фундамент) | 2× `WoodenLog` (блокуються, поки частина стоїть) | Лопата |
| `wall_down` (нижній ряд колод) | 2× `WoodenLog` + 10× `Nails` | Молоток / сокира |
| `wall_up` (верхній ряд) | 2× `WoodenLog` + `MetalWire` (блокується) | Молоток / сокира |
| `spikes` (загострені верхівки) | 10× `Nails` | Молоток / сокира |

* Якщо зняти мотузку з кіта — він розбирається на 4 палиці й мотузку (як ванільний).
* Якщо зруйновано `base` (розібрано лопатою) — частокіл зникає, на землі з'являється кіт.
* Поки побудовано лише кіт-місце без фундаменту, його можна скласти назад (`ActionFoldBaseBuildingObject`).
* Будувати/розбирати можна тільки з «лицьового» боку (як у ванільного паркану).
* Кожна частина має свою зону пошкоджень — при руйнуванні зони руйнується відповідна частина.

## Що потрібно зробити тобі (моделі)

Скрипти й конфіг готові, але в DayZ немає ванільної моделі частоколу, тому потрібні свої `.p3d`:

1. **`data/palisade.p3d`** — сама стіна.
   * Іменовані селекції `base`, `wall_down`, `wall_up`, `spikes` в усіх LOD-ах:
     Resolution, **Geometry**, **Fire Geometry**, **View Geometry** (щоб колізія з'являлась тільки коли частину збудовано).
   * Memory LOD-точки:
     * `wall_down_min`, `wall_down_max`, `wall_up_min`, `wall_up_max` — кути об'єму частини (перевірка, чи нічого не заважає будівництву, `collision_data`);
     * `base`, `wall_down`, `wall_up`, `spikes` — точки взаємодії (по ним перевіряється дистанція 2 м);
     * `kit_spawn_position` — де з'явиться кіт після розбирання (необов'язково).
   * Перед моделі має дивитися в напрямку осі +Z (бік, з якого будують).
   * `data/model.cfg` — вже готовий шаблон анімацій приховування для цих селекцій.
2. **`data/palisade_placing.p3d`** — модель голограми (той самий силует, одна селекція `placing`)
   + текстура `palisade_co.paa` і `palisade.rvmat`.
3. Модель кіта зараз — ванільна `\DZ\gear\camping\fence_kit.p3d` (купа палиць з мотузкою). Можеш замінити на свою.

Якщо твій мод має інший префікс/папку — заміни `PalisadeMod` у шляхах в `config.cpp`
(`files[] = {"PalisadeMod/scripts/4_World"}` і `\PalisadeMod\data\...`).

## Збірка й тест

1. Запакуй `PalisadeMod` у PBO (Addon Builder з DayZ Tools, префікс `PalisadeMod`).
2. Поклади в `@PalisadeMod/addons/`, підключи `-mod=@PalisadeMod` на сервері й клієнті.
3. Додай записи з `server/types.xml` у `db/types.xml` місії (інакше об'єкт не збережеться після рестарту).
4. Тест в адмін-консолі: `GetGame().GetPlayer().GetInventory().CreateInInventory("PalisadeKit");`

## Налаштування

* Кількість матеріалів — `quantity` у `class Construction` в `config.cpp`.
* Міцність частин — `hitpoints` у `class DamageZones` (назва зони = назва частини).
* Інструменти — `build_action_type` / `dismantle_action_type` (4 = лопата, 2 = молоток/сокира, як у ванільного паркану).
* Висота перевірки при розміщенні — `HeightCheckOverride()` у `PalisadeKit.c`.
