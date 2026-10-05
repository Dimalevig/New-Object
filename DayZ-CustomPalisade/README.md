# Custom Palisade (DayZ mod)

Vanilla-style wooden palisade construction. Developed stage by stage;
see the technical specification for the full plan.

Current stage: **1 — mod skeleton** (no gameplay content yet).

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
[CustomPalisade] INFO: Server loaded. Version 0.1.0, module CustomPalisade:4_World
```

Client (`-mod=@CustomPalisade`, join the server) — in the client `script_*.log`:

```
[CustomPalisade] INFO: Client loaded. Version 0.1.0, module CustomPalisade:4_World
```

Neither log (nor the RPT) may contain `Can't compile`, `Unknown type`,
or `CustomPalisade` errors.
