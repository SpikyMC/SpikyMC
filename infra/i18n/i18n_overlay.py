#!/usr/bin/env python3
import glob
import hashlib
import json
import os
import subprocess
import sys

DST_DIR = "/var/www/mc.spiky.team/i18n/"
OVERLAY_DIR = "/opt/spikymc-meta/i18n-overlay/"
WORK_DIR = "/opt/spikymc-meta/i18n-work/"
WORK_INDEX = os.path.join(WORK_DIR, "index_v2.json")
LCONVERT = "/usr/bin/lconvert"
LRELEASE = "/usr/bin/lrelease"


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def main():
    if not os.path.exists(WORK_INDEX):
        print("index not found:", WORK_INDEX)
        return 1
    index = json.load(open(WORK_INDEX))
    lang_map = index["languages"]

    if not os.path.isdir(OVERLAY_DIR):
        print("overlay dir missing:", OVERLAY_DIR)
        return 0

    ts_files = sorted(glob.glob(os.path.join(OVERLAY_DIR, "*.ts")))
    if not ts_files:
        print("no overlay .ts files in", OVERLAY_DIR)
        return 0

    changed = False
    for ts in ts_files:
        code = os.path.splitext(os.path.basename(ts))[0]
        if code not in lang_map:
            print("SKIP (language not in index):", code)
            continue
        info = lang_map[code]
        base_ts = os.path.join(WORK_DIR, code + "-base.ts")
        if not os.path.exists(base_ts):
            print("SKIP (base ts missing):", code, base_ts)
            continue

        merged_ts = "/tmp/overlay-{}.merged.ts".format(code)
        qm_tmp = "/tmp/overlay-{}.qm".format(code)

        r = run([LCONVERT, "-i", base_ts, ts, "-o", merged_ts])
        if r.returncode != 0:
            print("lconvert merge FAILED for", code, ":\n" + r.stderr)
            return 1
        r = run([LRELEASE, "-qm", qm_tmp, merged_ts])
        if r.returncode != 0:
            print("lrelease FAILED for", code, ":\n" + r.stderr)
            return 1

        data = open(qm_tmp, "rb").read()
        new_sha = hashlib.sha1(data).hexdigest()
        new_name = new_sha + ".class"
        old_path = os.path.join(DST_DIR, info["file"])
        if os.path.exists(old_path):
            os.remove(old_path)
        with open(os.path.join(DST_DIR, new_name), "wb") as f:
            f.write(data)
        info["file"] = new_name
        info["sha1"] = new_sha
        info["size"] = len(data)
        info["translated"] = info.get("translated", 0) + 1
        info["untranslated"] = max(0, info.get("untranslated", 0) - 2)
        changed = True
        print("OVERLAYED {} -> {} ({} bytes)".format(code, new_name, len(data)))

    if changed:
        with open(WORK_INDEX, "w") as f:
            json.dump(index, f, indent=4)
        os.replace(WORK_INDEX, os.path.join(DST_DIR, "index_v2.json"))
        print("index_v2.json updated")
    return 0


if __name__ == "__main__":
    sys.exit(main())