#!/usr/bin/env python3
"""Пакує Oblivion/ у @Oblivion/Addons/Oblivion.pbo (без бінаризації, без підпису).

Запуск:  python3 tools/pack.py
Результат: dist/@Oblivion/ і dist/Oblivion.zip
"""
import hashlib
import os
import shutil
import struct
import time
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "Oblivion")
DIST = os.path.join(ROOT, "dist")
MOD = os.path.join(DIST, "@Oblivion")
PREFIX = "Oblivion"

MOD_CPP = """name = "Oblivion";
author = "Oblivion Server";
version = "{version}";
"""


def entry(name: bytes, method: int, size: int, timestamp: int) -> bytes:
    # name\0, packing method, original size, reserved, timestamp, data size
    return name + b"\0" + struct.pack("<5I", method, 0, 0, timestamp, size)


def pack_pbo(src: str, out: str) -> int:
    files = []
    for root, _, names in os.walk(src):
        for n in sorted(names):
            full = os.path.join(root, n)
            rel = os.path.relpath(full, src).replace("/", "\\")
            files.append((rel, full))
    files.sort(key=lambda f: f[0].lower())

    header = bytearray()
    # Версійний запис із властивістю prefix.
    header += b"\0" + struct.pack("<5I", 0x56657273, 0, 0, 0, 0)
    header += b"prefix\0" + PREFIX.encode() + b"\0" + b"\0"

    data = bytearray()
    for rel, full in files:
        with open(full, "rb") as fh:
            content = fh.read()
        header += entry(rel.encode("utf-8"), 0, len(content), int(os.path.getmtime(full)))
        data += content
    header += entry(b"", 0, 0, 0)

    body = bytes(header) + bytes(data)
    with open(out, "wb") as fh:
        fh.write(body)
        fh.write(b"\0" + hashlib.sha1(body).digest())
    return len(files)


def read_version() -> str:
    with open(os.path.join(SRC, "Scripts", "3_Game", "Oblivion", "OblivionConstants.c"), encoding="utf-8") as fh:
        for line in fh:
            if "OBLIVION_MOD_VERSION" in line:
                return line.split('"')[1]
    return "0"


def main():
    if os.path.isdir(MOD):
        shutil.rmtree(MOD)
    os.makedirs(os.path.join(MOD, "Addons"))
    os.makedirs(os.path.join(MOD, "Keys"))

    count = pack_pbo(SRC, os.path.join(MOD, "Addons", "Oblivion.pbo"))
    version = read_version()
    with open(os.path.join(MOD, "mod.cpp"), "w", encoding="utf-8") as fh:
        fh.write(MOD_CPP.format(version=version))

    zip_path = os.path.join(DIST, "Oblivion.zip")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
        for root, dirs, names in os.walk(MOD):
            rel_root = os.path.relpath(root, DIST)
            z.write(root, rel_root)
            for n in names:
                z.write(os.path.join(root, n), os.path.join(rel_root, n))

    print(f"Oblivion v{version}: {count} файлів -> {os.path.relpath(MOD, ROOT)}, {os.path.relpath(zip_path, ROOT)}")


if __name__ == "__main__":
    main()
