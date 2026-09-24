#!/usr/bin/env bash
# Начальная настройка VPS для SpikyMC.
# Выполняется 1 раз от root на свежем Debian/Ubuntu.
#
# Настройки:
#   EMAIL      — почта для Let's Encrypt
set -euo pipefail

EMAIL="${1:-admin@spiky.team}"

echo "==> Обновление пакетов"
apt-get update -y
apt-get upgrade -y

echo "==> Установка nginx + certbot"
apt-get install -y nginx certbot python3-certbot-nginx rsync git curl wget

echo "==> Каталоги сайтов"
mkdir -p /var/www/mc.spiky.team/meta
mkdir -p /var/www/mc.spiky.team/i18n
mkdir -p /var/www/mc.spiky.team/fmllibs
mkdir -p /var/www/mc.spiky.team/wiki
mkdir -p /var/www/spiky.team
mkdir -p /opt/spikymc-deploy

echo "==> Дефолтный index.html (заглушки)"
cat > /var/www/mc.spiky.team/index.html <<'EOF'
<!DOCTYPE html><html><head><meta charset="utf-8"><title>SpikyMC</title></head>
<body><h1>SpikyMC</h1><p>Поддомен лаунчера. Сервисы: /meta/, /i18n/, /fmllibs/, /wiki/. Скачивания: github.com/SpikyTeam/SpikyMC/releases</p></body></html>
EOF
cat > /var/www/spiky.team/index.html <<'EOF'
<!DOCTYPE html><html><head><meta charset="utf-8"><title>Spiky Team</title></head>
<body><h1>Spiky Team</h1><p>Команда Spiky. Лаунчер: <a href="https://mc.spiky.team/">mc.spiky.team</a></p></body></html>
EOF

echo "==> Раздача конфигов nginx"
# После этого скрипта нужно вручную положить обновлённые конфиги:
#   infra/server/nginx/spiky.team.conf   -> /etc/nginx/sites-available/
#   infra/server/nginx/mc.spiky.team.conf-> /etc/nginx/sites-available/
#   и сделать симлинки в sites-enabled/
rm -f /etc/nginx/sites-enabled/default
nginx -t

echo "==> Получение сертификатов Let's Encrypt"
certbot --nginx \
  --email "$EMAIL" \
  --agree-tos \
  --no-eff-email \
  -d spiky.team -d www.spiky.team \
  -d mc.spiky.team \
  --redirect || true

echo "==> Cron обновления сертификатов"
cat > /etc/cron.d/certbot <<'EOF'
SHELL=/bin/sh
0 3 * * * root test -x /usr/bin/certbot && /usr/bin/certbot renew -q --post-hook "systemctl reload nginx"
EOF

echo "==> Готово"
echo "Положи конфиги nginx и перезапусти: systemctl reload nginx"
echo "Проверь DNS: A записи spiky.team и mc.spiky.team -> IP этого сервера"