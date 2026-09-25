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
#include <QHash>
#include <QByteArray>
#include <QJsonObject>

class QLocalSocket;
class QTimer;

/*!
 * Minimal Discord Rich Presence IPC client.
 *
 * Speaks the Discord IPC protocol over a local socket (windows named pipe / unix socket)
 * without any external dependency.
 */
class DiscordRPC : public QObject {
    Q_OBJECT
   public:
    enum class Role : unsigned char { None, Idle, Launching, Playing };

    /*!
     * All of the user-facing data shown in a "Now Playing" activity.
     */
    struct Activity {
        Role role = Role::None;
        QString state;    // second line, e.g. "Playing: <instance>"
        QString details;  // first line, e.g. "Minecraft"
        qint64 startTime = -1;  // epoch seconds, <0 hides elapsed timer
        QString largeImage;     // asset key from the developer portal
        QString largeText;      // tooltip for the large image
        QString smallImage;     // asset key from the developer portal
        QString smallText;      // tooltip for the small image
    };

    explicit DiscordRPC(QObject* parent = nullptr);
    ~DiscordRPC() override;

    /*! True if we are currently connected to a Discord client instance. */
    bool isConnected() const;

    /*! Set the application id shown in the "Now Playing" activity. */
    void setClientId(const QString& clientId);

    /*!
     * Update the current presence. Only re-sends when the values actually changed.
     * Pass startTime < 0 to hide the elapsed timer.
     */
    void changeActivity(const Activity& activity);

    /*! Disconnect and hide the presence entirely. */
    void shutdown();

   private:
    void connectToNetwork();
    void onSocketConnected();
    void onSocketError();
    void onReadyRead();
    void sendFrame(int opcode, const QJsonObject& payload);
    void sendHandshake();
    void sendActivity();
    void sendPing();
    void onHeartbeat();

    /*! Current presence. */
    Activity m_activity;

    QString m_clientId;
    QLocalSocket* m_socket = nullptr;
    QTimer* m_heartbeatTimer = nullptr;
    bool m_ready = false;
    bool m_disconnectAll = false;
    QByteArray m_recvBuffer;
    int m_currentPipe = 0;
    int m_connectionAttempts = 0;
};