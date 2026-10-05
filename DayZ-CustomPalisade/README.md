# Custom Palisade (DayZ mod)

Vanilla-style wooden palisade construction. Developed stage by stage;
see the technical specification for the full plan.

Current stage: **2 — palisade kit item** (`CP_PalisadeKit`) + test palisade wall (`CP_PalisadeWall`).

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
   - Files to copy directly: `*.c;*.csv;*.paa;*.rvmat;*.p3d` (default list + `*.c`, `*.csv`)
   - Exclude: `*.gitkeep`
   - Sign with your private key; copy the `.bikey` to `@CustomPalisade/Keys`.

## Stage 1 test

Server: `DayZServer_x64.exe -config=serverDZ.cfg -mod=@CustomPalisade -scrAllowFileWrite`
(or add to your existing `-mod=` list).

In `profiles/script_*.log` expect:

```
[CustomPalisade] INFO: Server loaded. Version 0.2.0, module CustomPalisade:4_World
```

Client (`-mod=@CustomPalisade`, join the server) — in the client `script_*.log`:

```
[CustomPalisade] INFO: Client loaded. Version 0.2.0, module CustomPalisade:4_World
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
