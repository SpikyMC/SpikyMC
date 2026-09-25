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

#include <QObject>

#include "discordrpc/DiscordRPC.h"

class BaseInstance;
class InstanceList;
class LaunchTask;

/*!
 * Tracks what the user is doing and pushes a fitting Discord activity.
 */
class DiscordPresenceManager : public QObject {
    Q_OBJECT
   public:
    explicit DiscordPresenceManager(const InstanceList* instances, QObject* parent = nullptr);

    void updateSettings();

   private:
    void onInstancesChanged();
    void connectInstance(BaseInstance* instance);
    void onRunningStatusChanged(bool running);
    void onLaunchTaskChanged(LaunchTask* task);
    void onTaskStatus(QString status);
    void onGameReady();
    void onTaskFinished();

    void updatePresence();
    void setLauncherPresence();
    DiscordRPC::Activity makeActivity(DiscordRPC::Role role, const QString& state, const QString& details, qint64 startTime) const;
    QString instanceVersion() const;

    const InstanceList* m_instances;
    DiscordRPC m_rpc;
    BaseInstance* m_activeInstance = nullptr;
    LaunchTask* m_activeTask = nullptr;
    QString m_launchState;
    qint64 m_launchStartTime = -1;
    bool m_launched = false;

    // user-visible configuration
    bool m_showLauncherPresence = true;
    bool m_showGamePresence = true;
    bool m_showInstanceName = true;
    bool m_showVersion = true;

    // configured art assets
    QString m_largeImage;
    QString m_largeText;
    QString m_smallImage;
    QString m_smallText;
};