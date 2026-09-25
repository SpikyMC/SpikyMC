// SPDX-License-Identifier: GPL-3.0-only
/*
 *  SpikyMC - Minecraft Launcher
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QWidget>

#include "ui/pages/BasePage.h"

class QCheckBox;
class QGroupBox;

class DiscordRPCPage : public QWidget, public BasePage {
    Q_OBJECT

   public:
    explicit DiscordRPCPage(QWidget* parent = 0);

    QString displayName() const override { return tr("Discord RPC"); }
    QIcon icon() const override { return QIcon::fromTheme("discord"); }
    QString id() const override { return "discord-rpc"; }
    QString helpPage() const override { return QString(); }
    bool apply() override;
    void retranslate() override;

   private:
    void applySettings();
    void loadSettings();
    void updateEnabled();

   private:
    QCheckBox* m_enabled;
    QGroupBox* m_options;
    QCheckBox* m_showLauncherPresence;
    QCheckBox* m_showGamePresence;
    QCheckBox* m_showInstanceName;
    QCheckBox* m_showVersion;
};