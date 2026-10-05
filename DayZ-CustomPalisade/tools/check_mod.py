#!/usr/bin/env python3
"""Static sanity checks for the CustomPalisade source tree.

This does NOT replace DayZ's compiler. It catches packaging/structure
mistakes before a PBO is built: unbalanced braces, script module paths
in config.cpp that do not exist, a missing $PBOPREFIX$, stringtable rows
with the wrong column count, and non-ASCII text in config files.
"""
import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ADDON = ROOT / "CustomPalisade"
MOD_CPP = ROOT / "@CustomPalisade" / "mod.cpp"

errors = []


def strip_comments_and_strings(text):
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r'"(?:[^"\\\n]|\\.)*"', '""', text)


def check_braces(path):
    code = strip_comments_and_strings(path.read_text(encoding="utf-8"))
    for open_ch, close_ch in (("{", "}"), ("(", ")"), ("[", "]")):
        depth = 0
        for line_no, line in enumerate(code.splitlines(), 1):
            for ch in line:
                if ch == open_ch:
                    depth += 1
                elif ch == close_ch:
                    depth -= 1
                    if depth < 0:
                        errors.append(f"{path}:{line_no}: unexpected '{close_ch}'")
                        return
        if depth != 0:
            errors.append(f"{path}: unbalanced '{open_ch}{close_ch}' (depth {depth})")


def check_ascii(path):
    for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if any(ord(c) > 127 for c in line):
            errors.append(f"{path}:{line_no}: non-ASCII character (use stringtable.csv)")


def check_config():
    config = ADDON / "config.cpp"
    if not config.is_file():
        errors.append("config.cpp is missing")
        return
    check_braces(config)
    check_ascii(config)
    text = config.read_text(encoding="utf-8")
    if "class CfgPatches" not in text or "class CfgMods" not in text:
        errors.append("config.cpp: CfgPatches/CfgMods missing")
    prefix_file = ADDON / "$PBOPREFIX$"
    prefix = prefix_file.read_text().strip() if prefix_file.is_file() else None
    if prefix is None:
        errors.append("$PBOPREFIX$ is missing")
    for module_path in re.findall(r'files\[\]\s*=\s*\{\s*"([^"]+)"', text):
        first, _, rest = module_path.partition("/")
        if first != prefix:
            errors.append(f"config.cpp: '{module_path}' does not start with PBO prefix '{prefix}'")
        target = ADDON / rest
        if not target.is_dir():
            errors.append(f"config.cpp: script module path '{module_path}' does not exist")
        elif not list(target.rglob("*.c")):
            errors.append(f"config.cpp: script module path '{module_path}' has no .c files")


def check_scripts():
    for script in sorted((ADDON / "Scripts").rglob("*.c")):
        check_braces(script)


def check_stringtable():
    path = ADDON / "stringtable.csv"
    if not path.is_file():
        errors.append("stringtable.csv is missing")
        return
    with path.open(encoding="utf-8", newline="") as f:
        rows = list(csv.reader(f))
    width = len(rows[0])
    for row_no, row in enumerate(rows, 1):
        if len(row) != width:
            errors.append(f"stringtable.csv:{row_no}: {len(row)} columns, expected {width}")


def main():
    if not MOD_CPP.is_file():
        errors.append("@CustomPalisade/mod.cpp is missing")
    else:
        check_ascii(MOD_CPP)
    check_config()
    check_scripts()
    check_stringtable()
    if errors:
        print("FAILED")
        for e in errors:
            print("  " + e)
        return 1
    print("OK: config, scripts, stringtable, mod.cpp")
    return 0


if __name__ == "__main__":
    sys.exit(main())
