#!/usr/bin/env python3
import hashlib
import json
import os
import subprocess
import sys

SRC_BASE = "https://i18n.prismlauncher.org/"
DST_DIR = "/var/www/mc.spiky.team/i18n/"
WORK_DIR = "/opt/spikymc-meta/i18n-work/"
INDEX = os.path.join(WORK_DIR, "index_v2.json")
MIN_PERCENT = 90.0

os.makedirs(DST_DIR, exist_ok=True)
os.makedirs(WORK_DIR, exist_ok=True)

subprocess.run(["curl", "-s", "-o", INDEX, SRC_BASE + "index_v2.json"], check=True)
data = json.load(open(INDEX))
print("file_type:", data["file_type"], "version:", data["version"])
langs = data["languages"]
all_count = len(langs)
print("total langs:", all_count)

ok = bad = 0
for code, info in langs.items():
    fname = info["file"]
    expected = info["sha1"].lower()
    size = info["size"]
    dest = os.path.join(DST_DIR, fname)
    subprocess.run(["curl", "-s", "-o", dest, SRC_BASE + fname], check=True)
    actual_size = os.path.getsize(dest)
    if actual_size != size:
        print("SIZE MISMATCH", code, fname, "expected", size, "got", actual_size)
        bad += 1
        continue
    h = hashlib.sha1(open(dest, "rb").read()).hexdigest()
    if h != expected:
        print("SHA MISMATCH", code, fname, "expected", expected, "got", h)
        bad += 1
        continue
    ok += 1
    print("OK", code, fname, size)

print("*" * 40)
print("verified", ok, "/", len(langs))
if bad:
    sys.exit(1)

keep = {}
for code, info in langs.items():
    total = info.get("translated", 0) + info.get("fuzzy", 0) + info.get("untranslated", 0)
    if total == 0:
        keep[code] = info
    elif info.get("translated", 0) * 100.0 / total >= MIN_PERCENT:
        keep[code] = info
    else:
        print("DROP (below {:.0f}%): {} ({:.1f}%)".format(MIN_PERCENT, code, info.get("translated", 0) * 100.0 / total))
data["languages"] = keep
with open(INDEX, "w") as f:
    json.dump(data, f, indent=4)
with open(os.path.join(DST_DIR, "index_v2.json"), "w") as f:
    json.dump(data, f, indent=4)
print("filtered languages:", len(keep), "/", all_count)

kept_names = {info["file"] for info in keep.values()}
removed = 0
for fname in os.listdir(DST_DIR):
    if not fname.endswith(".class"):
        continue
    if fname not in kept_names:
        os.remove(os.path.join(DST_DIR, fname))
        removed += 1
print("removed orphan files:", removed)

OVERLAY = os.path.join(os.path.dirname(os.path.abspath(__file__)), "i18n_overlay.py")
r = subprocess.run([sys.executable, OVERLAY])
if r.returncode != 0:
    sys.exit(r.returncode)