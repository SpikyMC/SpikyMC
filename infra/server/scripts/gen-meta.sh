#!/bin/bash
# SpikyMC meta generator
# Copy of /opt/spikymc-meta/gen-meta.sh on the server.
# Run: bash infra/server/scripts/gen-meta.sh (on the VPS via scp), or edit in place.
set -e

BASE=/opt/spikymc-meta
GENERATOR=$BASE/generator
DEPLOY=/var/www/mc.spiky.team/meta/v1

export META_CACHE_DIR=$BASE/cache
export META_UPSTREAM_DIR=$BASE/upstream
export META_LAUNCHER_DIR=$BASE/launcher

cd "$GENERATOR"
. .venv/bin/activate

mkdir -p "$META_UPSTREAM_DIR" "$META_LAUNCHER_DIR" "$META_CACHE_DIR"

echo "=== updating raw data from sources ==="
python -m meta.run.update_mojang || exit 1
python -m meta.run.update_forge || exit 1
python -m meta.run.update_neoforge || exit 1
python -m meta.run.update_fabric || exit 1
python -m meta.run.update_quilt || exit 1
python -m meta.run.update_liteloader || exit 1
python -m meta.run.update_java || exit 1

echo "=== generating launcher metadata ==="
python -m meta.run.generate_mojang || exit 1
python -m meta.run.generate_forge || exit 1
python -m meta.run.generate_neoforge || exit 1
python -m meta.run.generate_fabric || exit 1
python -m meta.run.generate_quilt || exit 1
python -m meta.run.generate_liteloader || exit 1
python -m meta.run.generate_java || exit 1
python -m meta.run.index || exit 1

echo "=== deploying to $DEPLOY ==="
rsync -a --delete --exclude=.git "$META_LAUNCHER_DIR/" "$DEPLOY/"
chown -R www-data:www-data "$DEPLOY"

# free disk: drop transient HTTP cache after a successful run
rm -rf "$META_CACHE_DIR"/*
echo "=== cache cleaned ==="
echo "=== done ==="