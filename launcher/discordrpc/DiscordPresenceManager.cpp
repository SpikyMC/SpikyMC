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

#include "DiscordPresenceManager.h"

#include <QDateTime>

#include "Application.h"
#include "BaseInstance.h"
#include "InstanceList.h"
#include "launch/LaunchTask.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "settings/SettingsObject.h"

DiscordPresenceManager::DiscordPresenceManager(const InstanceList* instances, QObject* parent)
    : QObject(parent), m_instances(instances)
{
    connect(m_instances, &InstanceList::instancesChanged, this, &DiscordPresenceManager::onInstancesChanged);

    updateSettings();
}

void DiscordPresenceManager::updateSettings()
{
    auto settings = APPLICATION->settings();
    bool enabled = settings->get("DiscordPresence").toBool();
    QString clientId = settings->get("DiscordClientId").toString();

    m_largeImage = settings->get("DiscordLargeImage").toString();
    m_largeText = settings->get("DiscordLargeImageText").toString();
    m_smallImage = settings->get("DiscordSmallImage").toString();
    m_smallText = settings->get("DiscordSmallImageText").toString();
    m_showInstanceName = settings->get("DiscordShowInstanceName").toBool();
    m_showVersion = settings->get("DiscordShowVersion").toBool();
    m_showLauncherPresence = settings->get("DiscordShowLauncherPresence").toBool();
    m_showGamePresence = settings->get("DiscordShowGamePresence").toBool();

    if (!enabled || clientId.isEmpty()) {
        m_rpc.shutdown();
        return;
    }

    m_rpc.setClientId(clientId);
    updatePresence();
}

void DiscordPresenceManager::onInstancesChanged()
{
    for (int i = 0; i < m_instances->count(); ++i) {
        connectInstance(m_instances->at(i));
    }
}

void DiscordPresenceManager::connectInstance(BaseInstance* instance)
{
    connect(instance, &BaseInstance::runningStatusChanged, this, &DiscordPresenceManager::onRunningStatusChanged, Qt::UniqueConnection);
    connect(instance, &BaseInstance::launchTaskChanged, this, &DiscordPresenceManager::onLaunchTaskChanged, Qt::UniqueConnection);
}

void DiscordPresenceManager::onRunningStatusChanged(bool running)
{
    auto* instance = qobject_cast<BaseInstance*>(sender());
    if (!instance)
        return;

    if (running) {
        Q_ASSERT(!m_activeInstance || m_activeInstance == instance);
        m_activeInstance = instance;
    } else if (m_activeInstance == instance) {
        m_activeInstance = nullptr;
        m_activeTask = nullptr;
    }

    if (!m_activeTask) {
        m_launched = false;
        m_launchState.clear();
        m_launchStartTime = -1;
    }

    updatePresence();
}

void DiscordPresenceManager::onLaunchTaskChanged(LaunchTask* task)
{
    auto* instance = qobject_cast<BaseInstance*>(sender());
    if (!instance)
        return;

    if (task != m_activeTask) {
        if (m_activeTask) {
            disconnect(m_activeTask, &LaunchTask::status, this, &DiscordPresenceManager::onTaskStatus);
            disconnect(m_activeTask, &LaunchTask::readyForLaunch, this, &DiscordPresenceManager::onGameReady);
            disconnect(m_activeTask, &LaunchTask::succeeded, this, &DiscordPresenceManager::onTaskFinished);
            disconnect(m_activeTask, &LaunchTask::failed, this, &DiscordPresenceManager::onTaskFinished);
            disconnect(m_activeTask, &LaunchTask::aborted, this, &DiscordPresenceManager::onTaskFinished);
        }

        m_activeTask = task;
        m_launchState.clear();
        m_launched = false;
        m_launchStartTime = -1;

        if (m_activeTask) {
            connect(m_activeTask, &LaunchTask::status, this, &DiscordPresenceManager::onTaskStatus, Qt::UniqueConnection);
            connect(m_activeTask, &LaunchTask::readyForLaunch, this, &DiscordPresenceManager::onGameReady, Qt::UniqueConnection);
            connect(m_activeTask, &LaunchTask::succeeded, this, &DiscordPresenceManager::onTaskFinished, Qt::UniqueConnection);
            connect(m_activeTask, &LaunchTask::failed, this, &DiscordPresenceManager::onTaskFinished, Qt::UniqueConnection);
            connect(m_activeTask, &LaunchTask::aborted, this, &DiscordPresenceManager::onTaskFinished, Qt::UniqueConnection);
            if (m_activeInstance == instance) {
                m_launchStartTime = -1;
            }
        }
    }

    updatePresence();
}

void DiscordPresenceManager::onTaskStatus(QString status)
{
    auto* task = qobject_cast<LaunchTask*>(sender());
    if (task == m_activeTask) {
        m_launchState = std::move(status);
        updatePresence();
    }
}

void DiscordPresenceManager::onGameReady()
{
    auto* task = qobject_cast<LaunchTask*>(sender());
    if (task == m_activeTask) {
        m_launched = true;
        if (m_launchStartTime < 0) {
            m_launchStartTime = QDateTime::currentSecsSinceEpoch();
        }
        updatePresence();
    }
}

void DiscordPresenceManager::onTaskFinished()
{
    auto* task = qobject_cast<LaunchTask*>(sender());
    if (task != m_activeTask)
        return;

    m_activeInstance = nullptr;
    m_activeTask = nullptr;
    m_launched = false;
    m_launchState.clear();
    m_launchStartTime = -1;

    updatePresence();
}

void DiscordPresenceManager::updatePresence()
{
    // "browsing the launcher" presence
    if (!m_activeInstance || !m_activeTask) {
        if (m_showLauncherPresence) {
            setLauncherPresence();
        } else {
            m_rpc.shutdown();
        }
        return;
    }

    // game-related presence
    if (!m_showGamePresence) {
        if (m_showLauncherPresence) {
            setLauncherPresence();
        } else {
            m_rpc.shutdown();
        }
        return;
    }

    const QString instanceName = m_activeInstance->name();
    const DiscordRPC::Role role = m_launched ? DiscordRPC::Role::Playing : DiscordRPC::Role::Launching;

    // Best practice: keep strings short.
    //   details = first line (the game/context)
    //   state   = second line (what the player is doing)
    QString details = m_launched ? tr("Minecraft") : (m_showInstanceName ? instanceName : tr("Minecraft"));
    if (m_showVersion) {
        const QString version = instanceVersion();
        if (!version.isEmpty()) {
            details += QStringLiteral(" ") + version;
        }
    }

    if (m_launched) {
        const QString state = m_showInstanceName ? tr("Playing %1").arg(instanceName) : tr("Playing");
        m_rpc.changeActivity(makeActivity(role, state, details, m_launchStartTime));
    } else {
        const QString state = m_launchState.isEmpty() ? tr("Launching...") : m_launchState;
        m_rpc.changeActivity(makeActivity(role, state, details, -1));
    }
}

void DiscordPresenceManager::setLauncherPresence()
{
    m_rpc.changeActivity(makeActivity(DiscordRPC::Role::Idle, tr("Browsing the launcher"), tr("SpikyMC"), -1));
}

QString DiscordPresenceManager::instanceVersion() const
{
    if (!m_activeInstance)
        return QString();
    auto* mcInstance = dynamic_cast<MinecraftInstance*>(m_activeInstance);
    if (!mcInstance)
        return QString();
    auto* components = mcInstance->getPackProfile();
    if (!components)
        return QString();
    return components->getComponentVersion("net.minecraft");
}

DiscordRPC::Activity DiscordPresenceManager::makeActivity(DiscordRPC::Role role,
                                                         const QString& state,
                                                         const QString& details,
                                                         qint64 startTime) const
{
    DiscordRPC::Activity activity;
    activity.role = role;
    activity.state = state;
    activity.details = details;
    activity.startTime = startTime;
    activity.largeImage = m_largeImage;
    activity.largeText = m_largeText;
    activity.smallImage = m_smallImage;
    activity.smallText = m_smallText;
    return activity;
}