# Custom Palisade (DayZ mod)

Vanilla-style wooden palisade construction. Developed stage by stage;
see the technical specification for the full plan.

Current stage: **2 — palisade kit item** (`CP_PalisadeKit`) + test palisade wall (`CP_PalisadeWall`)
+ the mod's own log palisade model on a static test object (`CP_PalisadeWallModel`).

> The wall temporarily reuses the vanilla Fence model (wooden variant) and
> its vanilla construction/dismantle actions. The mod's own log-palisade
> model and construction stages come in later stages.

## Layout

```
DayZ-CustomPalisade/
├── @CustomPalisade/          # what goes into the DayZ / server folder
│   ├── mod.cpp               # launcher metadata (must NOT be inside the PBO)
│   ├── Addons/               # put the built CustomPalisade.pbo (+ .bisign) here
│   └── Keys/                 # put the .bikey here
├── CustomPalisade/           # PBO source — pack this folder
│   ├── $PBOPREFIX$           # prefix = CustomPalisade
│   ├── config.cpp
│   ├── stringtable.csv
│   ├── Scripts/
│   │   ├── 3_Game/CustomPalisade/
│   │   ├── 4_World/CustomPalisade/
│   │   └── 5_Mission/CustomPalisade/
│   └── Data/{Models,Textures,Materials}/
├── ServerFiles/
│   └── types_CustomPalisade.xml  # add to the mission (see below)
└── tools/check_mod.py        # static pre-pack checks
```

## Build

1. `python3 tools/check_mod.py` — must print `OK`.
2. DayZ Tools → Addon Builder:
   - Source: `DayZ-CustomPalisade/CustomPalisade`
   - Destination: `@CustomPalisade/Addons`
   - Prefix: `CustomPalisade`
   - Files to copy directly: `*.c;*.csv;*.xml` (models are binarised, see below)
   - Exclude: `*.gitkeep`
   - Sign with your private key; copy the `.bikey` to `@CustomPalisade/Keys`.

## Stage 1 test

Server: `DayZServer_x64.exe -config=serverDZ.cfg -mod=@CustomPalisade -scrAllowFileWrite`
(or add to your existing `-mod=` list).

In `profiles/script_*.log` expect:

```
[CustomPalisade] INFO: Server loaded. Version 0.2.1, module CustomPalisade:4_World
```

Client (`-mod=@CustomPalisade`, join the server) — in the client `script_*.log`:

```
[CustomPalisade] INFO: Client loaded. Version 0.2.1, module CustomPalisade:4_World
```

Neither log (nor the RPT) may contain `Can't compile`, `Unknown type`,
or `CustomPalisade` errors.

## Server economy (types.xml)

So the central economy does not clean up the wall, register the types file
in your mission (e.g. `mpmissions/dayzOffline.chernarusplus`):

1. Copy `ServerFiles/types_CustomPalisade.xml` to `<mission>/custom/`.
2. In `<mission>/cfgeconomycore.xml`, inside `<economycore>`, add:

```xml
<ce folder="custom">
    <file name="types_CustomPalisade.xml" type="types" />
</ce>
```

## Test wall

`CP_DEBUG_SPAWN_TEST_WALL = true` in
`Scripts/3_Game/CustomPalisade/CP_Constants.c` makes the server spawn one
finished wooden palisade wall ~4 m in front of a player about 5 s after
they join, unless a palisade wall already exists within 30 m.
Expected server log line:

```
[CustomPalisade] INFO: Palisade wall built at <pos>, parts: N
```

Admin tools can also spawn `CP_PalisadeWall` by class name; "spawn special"
(OnDebugSpawn) builds the wooden wall, a plain spawn gives an empty base site.

Set the flag to `false` for a release build.

## Test kit (stage 2)

`CP_DEBUG_SPAWN_TEST_KIT = true` drops one `CP_PalisadeKit` ("Розмітка частоколу")
at the player's feet ~5 s after joining, unless a kit already lies within 5 m.
Expected server log line:

```
[CustomPalisade] INFO: Palisade kit spawned at <pos>
```

In stage 2 the kit can be picked up, carried, dropped and disassembled
(detach the rope). Placing it is disabled until stage 3.

## Palisade model (`Data/Models/cp_palisade_wall.p3d`)

Author's model, retextured to **vanilla DayZ textures only** (paths verified
against the game's data):

| part   | texture | material |
|--------|---------|----------|
| logs   | `dz\gear\consumables\data\pile_of_planks_co.paa` (bark column) | `dz\gear\camping\data\wooden_log.rvmat` |
| planks | `dz\gear\consumables\data\wooden_planks_co.paa` | `dz\gear\consumables\data\wooden_planks.rvmat` |
| wire   | `dz\gear\crafting\data\string_metalwire_co.paa` | `dz\gear\camping\data\fence_metalwire.rvmat` |
| Fire Geometry | – | `dz\data\data\penetration\wood_desk.rvmat` |

No game files are shipped in the mod. Pipeline (in `tools/model/`):

1. `cp_p3d_retexture.py in.p3d out.p3d` – texture/material paths + UV remap, LOD count fix;
   everything else copied byte for byte.
2. `cp_fix_model_blender.py -- in.p3d out.p3d` (Blender + Arma 3 Object Builder) –
   removes duplicated faces, turns inside-out wire faces outwards, recomputes normals.

Selections `base`, `wall_down`, `wall_up`, `spikes` are prepared in `model.cfg` as hide
animations (sources in `config.cpp`) for the construction stages.

**The model must be binarised**: Addon Builder → Options → *Binarize* ON, with the DayZ
Tools work drive **P:** mounted (game data extracted) and the mod source at
`P:\CustomPalisade`. Do **not** put `*.p3d`, `*.rvmat` or `model.cfg` in
"files to copy directly" (only `*.c;*.csv;*.xml`).
