# Инфраструктура SpikyMC — сервер

Раздача статики лаунчера на VPS (регион: РФ) через nginx + Let's Encrypt.
Инфраструктура самодостаточна: генератор меты, переводы, FML-библиотеки,
maven-артефакты, вики и лендинги живут на своём сервере. GitHub используется
только как репозиторий кода и источник релизов лаунчера (updater) — **не раздаёт**
дистрибутивы/мету в продакшене.

## Топология доменов

| Домен            | Назначение                                      |
| ---------------- | ------------------------------------------------ |
| `spiky.team`     | Лендинг команды, ссылки на проекты               |
| `mc.spiky.team`  | Лаунчер: сайт, мета, i18n, fmllibs, maven, вики, скачивания |

> `www.spiky.team` A-запись **не заведена** (SAN-сертификат только на
> `spiky.team` и `mc.spiky.team`). Если нужен www — добавить A и перевыпустить
> сертификат с доменом.

## Пути на mc.spiky.team

- `https://mc.spiky.team/meta/v1/` — мета-сервис (генерируется на сервере)
- `https://mc.spiky.team/i18n/` — переводы (`index_v2.json` + `<sha1>.class`)
- `https://mc.spiky.team/fmllibs/` — legacy FML библиотеки (MC <= 1.5.2)
- `https://mc.spiky.team/maven/` — патченные артефакты Prism (ForgeWrapper, log4j 2.0-beta9-fixed)
- `https://mc.spiky.team/wiki/` — вики/справка лаунчера (`help-pages/<page>`)
- `https://mc.spiky.team/downloads/` — редирект на GitHub releases (`SpikyTeam/SpikyMC/releases`)
- `https://mc.spiky.team/` — лендинг загрузки лаунчера

## Чек-лист настройки нового VPS

1. DNS: добавить A-записи `spiky.team`, `mc.spiky.team` → IP сервера.
2. Скачать скрипты и конфиги:
   ```bash
   scp infra/server/scripts/setup-server.sh root@<ip>:/
   scp infra/server/nginx/*.conf root@<ip>:/etc/nginx/sites-available/
   ```
3. Выполнить как root:
   ```bash
   chmod +x setup-server.sh && ./setup-server.sh admin@spiky.team
   ```
   (настроит пакеты, каталоги, SSH-порт, fail2ban, unattended-upgrades, UFW, certbot)
4. Включить конфиги:
   ```bash
   ln -sf /etc/nginx/sites-available/spiky.team.conf    /etc/nginx/sites-enabled/
   ln -sf /etc/nginx/sites-available/mc.spiky.team.conf /etc/nginx/sites-enabled/
   nginx -t && systemctl reload nginx
   ```
5. Развернуть данные (см. ниже: i18n/fmllibs/maven/wiki) и генератор меты.
6. Задеплоить лендинги/конфиги: `infra/server/scripts/deploy.sh user@<ip>`.

## Мета (`/meta/v1/`)

Генерируется на сервере форком **SpikyTeam/meta** (исходник PrismLauncher/meta),
данные тянутся напрямую из первоисточников (Mojang/Forge/Fabric/Quilt/Java API),
без обращения к Prism.

- Путь: `/opt/spikymc-meta/generator` (`.venv` с зависимостями).
- Запуск: `bash /opt/spikymc-meta/gen-meta.sh` — обновляет все источники,
  генерирует тексты и `index.json`, затем `rsync` в `/var/www/mc.spiky.team/meta/v1/`.
  Копия скрипта — `infra/server/scripts/gen-meta.sh`.
- Логи: `/opt/spikymc-meta/gen.log`.
- Первый прогон долгий (Forge перебирается последовательно, десятки минут).
- После успешного прогона `gen-meta.sh` сам удаляет транзиентный HTTP-кэш
  (`/opt/spikymc-meta/cache/*`, десятки ГБ) — диск не забивается между обновлениями.
- Внимание, на 2 ГБ RAM: не запускать `python -m meta.run.*` вручную без
  `META_CACHE_DIR`/`META_UPSTREAM_DIR` — иначе данные пишутся внутрь пакета
  (`generator/upstream`, `generator/cache`) и дублируют место. Форк также содержит
  два патча: `ThreadPoolExecutor(max_workers=2)` (анти-OOM на фазах качалок) и
  пропуск битых zip-архивов Forge внутрь `update_forge.py` вместо падения.
- Автоматизация: cron/systemd-timer, например ежедневно в 02:00:
  ```bash
  0 2 * * * root bash /opt/spikymc-meta/gen-meta.sh >> /opt/spikymc-meta/gen.log 2>&1
  ```

В генераторе `LAUNCHER_MAVEN` заменён на `https://mc.spiky.team/maven/%s`.
Заметка: лаунчер ходит в `meta/v1` относительно `Launcher_META_URL` (см.
`CMakeLists.txt`, переменная `Launcher_META_URL`).

## Переводы (`/i18n/`)

Слепок с `i18n.prismlauncher.org`: те же имена `<sha1>.class` (скомпилированные
QM) и `index_v2.json` (`file_type: MMC-TRANSLATION-INDEX`). Список и sha1
совпадают build-to-build, поэтому залит как статика; при желании добавить
свои строки/ключи — сверять словарь с `PrismLauncher/PrismLauncher` и
пере-генеровать `.qm`.

Синхронизация: `python3 /opt/spikymc-meta/i18n_sync.py` (качает с
`i18n.prismlauncher.org`, проверяет sha1, складывает в `/var/www/.../i18n/`).

## FML-библиотеки (`/fmllibs/`)

12 файлов из `launcher/minecraft/VersionFilterData.cpp` (argo, guava, asm-all,
bcprov, deobfuscation_data, scala-library), скачаны с `files.prismlauncher.org`,
sha1 совпадают с хардкодом. Дальнейшие обновления — сверять с этим списком.

## Maven (`/maven/`)

Патченные Prism-артефакты, которых нет в стандартных репозиториях:
`io.github.zekerzhayard/ForgeWrapper` (prism-2026-08-01) и
`org/apache/logging/log4j/log4j-{api,core}/2.0-beta9-fixed`.
Генератор ссылается на них через `LAUNCHER_MAVEN`.

## Вики (`/wiki/`)

Статическая: `index.html` + `help-pages/<page>.html`. Набор pageId = те, что
лаунчер открывает через `Launcher_HELP_URL` (см. `.arg(pageId)` в исходниках).
Генерация: `python3 /opt/spikymc-meta/gen_wiki.py`.

## Обновления лаунчера

Updater (`SpikyMCUpdater`) работает через `api.github.com` на релизах
`SpikyTeam/SpikyMC` — **от Prism не зависит**. Возможное улучшение для РФ:
свой `mc.spiky.team/updates.json` + fallback на GitHub.
Трекер в коде: `launcher/updater/spikymcupdater/SpikyMCUpdater.cpp` (`loadReleaseList`).