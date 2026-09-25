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

#include "DiscordRPCPage.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "Application.h"
#include "discordrpc/DiscordPresenceManager.h"
#include "settings/SettingsObject.h"

DiscordRPCPage::DiscordRPCPage(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("discordRpcPage"));

    m_enabled = new QCheckBox(tr("Enable Discord RPC (Rich Presence)"), this);

    m_options = new QGroupBox(tr("What to show"), this);
    m_showLauncherPresence = new QCheckBox(tr("Show status while browsing the launcher"), m_options);
    m_showGamePresence = new QCheckBox(tr("Show status while playing"), m_options);
    m_showInstanceName = new QCheckBox(tr("Show instance name"), m_options);
    m_showVersion = new QCheckBox(tr("Show instance version"), m_options);

    auto optionsLayout = new QVBoxLayout(m_options);
    optionsLayout->addWidget(m_showLauncherPresence);
    optionsLayout->addWidget(m_showGamePresence);
    optionsLayout->addWidget(m_showInstanceName);
    optionsLayout->addWidget(m_showVersion);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_enabled);
    layout->addWidget(m_options);
    layout->addStretch(1);

    connect(m_enabled, &QCheckBox::toggled, this, &DiscordRPCPage::updateEnabled);

    loadSettings();
}

bool DiscordRPCPage::apply()
{
    applySettings();
    return true;
}

void DiscordRPCPage::applySettings()
{
    auto settings = APPLICATION->settings();
    settings->set("DiscordPresence", m_enabled->isChecked());
    settings->set("DiscordShowLauncherPresence", m_showLauncherPresence->isChecked());
    settings->set("DiscordShowGamePresence", m_showGamePresence->isChecked());
    settings->set("DiscordShowInstanceName", m_showInstanceName->isChecked());
    settings->set("DiscordShowVersion", m_showVersion->isChecked());

    auto presence = APPLICATION->discordPresence();
    if (presence) {
        presence->updateSettings();
    }
}

void DiscordRPCPage::loadSettings()
{
    auto settings = APPLICATION->settings();
    m_enabled->setChecked(settings->get("DiscordPresence").toBool());
    m_showLauncherPresence->setChecked(settings->get("DiscordShowLauncherPresence").toBool());
    m_showGamePresence->setChecked(settings->get("DiscordShowGamePresence").toBool());
    m_showInstanceName->setChecked(settings->get("DiscordShowInstanceName").toBool());
    m_showVersion->setChecked(settings->get("DiscordShowVersion").toBool());
    updateEnabled();
}

void DiscordRPCPage::updateEnabled()
{
    bool enabled = m_enabled->isChecked();
    m_options->setEnabled(enabled);
}

void DiscordRPCPage::retranslate()
{
    m_enabled->setText(tr("Enable Discord RPC (Rich Presence)"));
    m_options->setTitle(tr("What to show"));
    m_showLauncherPresence->setText(tr("Show status while browsing the launcher"));
    m_showGamePresence->setText(tr("Show status while playing"));
    m_showInstanceName->setText(tr("Show instance name"));
    m_showVersion->setText(tr("Show instance version"));
}