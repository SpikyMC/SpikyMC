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

#include "DiscordRPC.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTimer>

// Discord IPC frame opcodes
static constexpr int OP_HANDSHAKE = 0;
static constexpr int OP_FRAME = 1;
static constexpr int OP_PING = 3;

// Deliberately not a huge number: localized socket names use 0..9
static constexpr int MAX_PIPE_TRIES = 10;

DiscordRPC::DiscordRPC(QObject* parent) : QObject(parent)
{
    m_socket = new QLocalSocket(this);
    connect(m_socket, &QLocalSocket::connected, this, &DiscordRPC::onSocketConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &DiscordRPC::onSocketError);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &DiscordRPC::onSocketError);
    connect(m_socket, &QLocalSocket::readyRead, this, &DiscordRPC::onReadyRead);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(15000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &DiscordRPC::onHeartbeat);
}

DiscordRPC::~DiscordRPC()
{
    shutdown();
}

bool DiscordRPC::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState && m_ready;
}

void DiscordRPC::setClientId(const QString& clientId)
{
    if (m_clientId == clientId)
        return;
    m_clientId = clientId;
    if (m_clientId.isEmpty()) {
        shutdown();
    }
}

void DiscordRPC::changeActivity(const Activity& activity)
{
    if (m_activity.role == activity.role && m_activity.state == activity.state && m_activity.details == activity.details &&
        m_activity.startTime == activity.startTime && m_activity.largeImage == activity.largeImage &&
        m_activity.largeText == activity.largeText && m_activity.smallImage == activity.smallImage &&
        m_activity.smallText == activity.smallText)
        return;

    m_activity = activity;

    if (m_clientId.isEmpty() || m_activity.role == Role::None) {
        shutdown();
        return;
    }

    if (m_socket->state() != QLocalSocket::ConnectedState) {
        m_connectionAttempts = 0;
        connectToNetwork();
    } else if (m_ready) {
        sendActivity();
    }
}

void DiscordRPC::connectToNetwork()
{
    if (m_clientId.isEmpty() || m_activity.role == Role::None) {
        return;
    }
    if (m_socket->state() == QLocalSocket::ConnectingState || m_socket->state() == QLocalSocket::ConnectedState) {
        return;
    }

    QString name;
#ifdef Q_OS_WIN
    name = QStringLiteral("\\\\.\\pipe\\discord-ipc-%1").arg(m_currentPipe);
#else
    {
        auto runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtime.isEmpty())
            runtime = QStringLiteral("/tmp");
        name = runtime + QStringLiteral("/discord-ipc-%1").arg(m_currentPipe);
    }
#endif
    m_recvBuffer.clear();
    m_ready = false;

    qDebug() << "DiscordRPC: connecting to" << name;
    m_socket->connectToServer(name);
}

void DiscordRPC::onSocketConnected()
{
    sendHandshake();
}

void DiscordRPC::onSocketError()
{
    m_ready = false;
    if (m_disconnectAll || m_clientId.isEmpty() || m_activity.role == Role::None) {
        m_currentPipe = 0;
        m_connectionAttempts = 0;
        m_socket->abort();
        return;
    }

    // Only try a handful of pipes per round; Discord isn't running when they all fail.
    if (m_connectionAttempts >= MAX_PIPE_TRIES) {
        m_currentPipe = 0;
        m_connectionAttempts = 0;
        m_socket->abort();
        // retry later on the heartbeat
        if (!m_heartbeatTimer->isActive()) {
            m_heartbeatTimer->start();
        }
        return;
    }

    m_currentPipe = (m_currentPipe + 1) % MAX_PIPE_TRIES;
    m_connectionAttempts++;
    connectToNetwork();
}

void DiscordRPC::sendFrame(int opcode, const QJsonObject& payload)
{
    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QByteArray frame;
    frame.reserve(8 + payloadBytes.size());
    frame.append(static_cast<char>(opcode & 0xFF));
    frame.append(static_cast<char>((opcode >> 8) & 0xFF));
    frame.append(static_cast<char>((opcode >> 16) & 0xFF));
    frame.append(static_cast<char>((opcode >> 24) & 0xFF));
    int payloadSize = payloadBytes.size();
    frame.append(static_cast<char>(payloadSize & 0xFF));
    frame.append(static_cast<char>((payloadSize >> 8) & 0xFF));
    frame.append(static_cast<char>((payloadSize >> 16) & 0xFF));
    frame.append(static_cast<char>((payloadSize >> 24) & 0xFF));
    frame.append(payloadBytes);

    m_socket->write(frame);
}

void DiscordRPC::sendHandshake()
{
    QJsonObject payload;
    payload.insert(QStringLiteral("v"), 1);
    payload.insert(QStringLiteral("client_id"), m_clientId);
    sendFrame(OP_HANDSHAKE, payload);
}

void DiscordRPC::sendActivity()
{
    if (!m_ready)
        return;

    QJsonObject activity;

    // Keep strings short and on one line: state = what they are doing, details = context.
    switch (m_activity.role) {
        case Role::Idle:
            activity.insert(QStringLiteral("state"), tr("Browsing the launcher"));
            activity.insert(QStringLiteral("details"), tr("SpikyMC"));
            break;
        case Role::Launching:
            activity.insert(QStringLiteral("state"), tr("Launching..."));
            activity.insert(QStringLiteral("details"), m_activity.details);
            break;
        case Role::Playing:
            activity.insert(QStringLiteral("state"), m_activity.state);
            activity.insert(QStringLiteral("details"), m_activity.details);
            break;
        case Role::None:
            return;
    }

    if (m_activity.startTime >= 0) {
        QJsonObject timestamps;
        timestamps.insert(QStringLiteral("start"), m_activity.startTime);
        activity.insert(QStringLiteral("timestamps"), timestamps);
    }

    // Art assets: large image is shared across players, small image can be per-player.
    if (!m_activity.largeImage.isEmpty()) {
        QJsonObject assets;
        assets.insert(QStringLiteral("large_image"), m_activity.largeImage);
        if (!m_activity.largeText.isEmpty()) {
            assets.insert(QStringLiteral("large_text"), m_activity.largeText);
        }
        if (!m_activity.smallImage.isEmpty()) {
            assets.insert(QStringLiteral("small_image"), m_activity.smallImage);
            if (!m_activity.smallText.isEmpty()) {
                assets.insert(QStringLiteral("small_text"), m_activity.smallText);
            }
        }
        activity.insert(QStringLiteral("assets"), assets);
    }

    QJsonObject args;
    args.insert(QStringLiteral("pid"), static_cast<int>(QCoreApplication::applicationPid()));
    args.insert(QStringLiteral("activity"), activity);

    QJsonObject payload;
    payload.insert(QStringLiteral("cmd"), QStringLiteral("SET_ACTIVITY"));
    payload.insert(QStringLiteral("args"), args);
    payload.insert(QStringLiteral("nonce"), QStringLiteral("activity-%1").arg(QDateTime::currentMSecsSinceEpoch()));

    sendFrame(OP_FRAME, payload);
}

void DiscordRPC::sendPing()
{
    QJsonObject payload;
    sendFrame(OP_PING, payload);
}

void DiscordRPC::onReadyRead()
{
    m_recvBuffer.append(m_socket->readAll());

    while (m_recvBuffer.size() >= 8) {
        int opcode = 0;
        int payloadSize = 0;
        for (int i = 0; i < 4; ++i) {
            opcode |= static_cast<unsigned char>(m_recvBuffer[i]) << (8 * i);
            payloadSize |= static_cast<unsigned char>(m_recvBuffer[i + 4]) << (8 * i);
        }
        if (payloadSize < 0 || m_recvBuffer.size() < 8 + payloadSize)
            break;

        QByteArray payloadBytes = m_recvBuffer.mid(8, payloadSize);
        m_recvBuffer.remove(0, 8 + payloadSize);

        QJsonParseError parseError;
        auto doc = QJsonDocument::fromJson(payloadBytes, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            continue;
        }
        auto payloadObj = doc.object();

        if (opcode == OP_FRAME) {
            auto cmd = payloadObj.value(QStringLiteral("cmd")).toString();
            if (cmd == QStringLiteral("DISPATCH")) {
                auto evt = payloadObj.value(QStringLiteral("evt")).toString();
                if (evt == QStringLiteral("READY")) {
                    m_ready = true;
                    m_currentPipe = 0;
                    qDebug() << "DiscordRPC: READY";
                    sendActivity();
                    m_heartbeatTimer->start();
                }
            }
        } else if (opcode == OP_PING) {
            sendPing();
        }
    }
}

void DiscordRPC::onHeartbeat()
{
    if (isConnected()) {
        sendActivity();
    } else {
        // Discord may have started (or the pipe may have opened) while we were idle.
        m_connectionAttempts = 0;
        connectToNetwork();
    }
}

void DiscordRPC::shutdown()
{
    m_disconnectAll = true;
    m_activity = Activity();
    m_heartbeatTimer->stop();
    if (m_socket->state() != QLocalSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_disconnectAll = false;
    m_currentPipe = 0;
}