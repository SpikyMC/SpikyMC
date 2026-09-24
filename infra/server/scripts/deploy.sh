#!/usr/bin/env bash
# Деплой статики на сервер. Запускается ЛОКАЛЬНО (с машины сборки/CI).
#
# Использование:
#   ./deploy.sh <user@server-ip> [--dry-run]
#
# Что кладёт:
#   infra/server/www/<domain>/  -> /var/www/<domain>/  лендинги
#   infra/server/nginx/*.conf   -> /etc/nginx/sites-available/
#
# Дистрибутивы лаунчера раздаются через GitHub releases (https://github.com/SpikyTeam/SpikyMC/releases).
# Мета/i18n/fmllibs/maven/wiki генерируются НА СЕРВЕРЕ скриптами из
# /opt/spikymc-meta/ (см. infra/server/README.md) — тут не трогаются.
set -euo pipefail

TARGET="${1:?Usage: ./deploy.sh <user@host> [--dry-run]}"
DRY="${2:-}"

REMOTE_ROOT="/var/www/mc.spiky.team"

RSYNC_OPTS="-avz --delete --partial"
if [[ "$DRY" == "--dry-run" ]]; then
    RSYNC_OPTS="$RSYNC_OPTS --dry-run"
fi

echo "==> Деплой на $TARGET"

# Лендинги сайтов
for site in spiky.team mc.spiky.team; do
    if [[ -d "www/$site" ]]; then
        echo "==> лендинг www/$site -> $REMOTE_ROOT"
        rsync $RSYNC_OPTS -e ssh "www/$site/" "$TARGET:$REMOTE_ROOT/"
    fi
done

# Конфиги nginx (не активируются автоматически — nginx -t && reload вручную)
if [[ -d "nginx" ]]; then
    echo "==> nginx/*.conf -> /etc/nginx/sites-available/"
    rsync -avz --partial -e ssh "nginx/" "$TARGET:/etc/nginx/sites-available/"
fi

echo "==> Готово. На сервере: nginx -t && systemctl reload nginx"