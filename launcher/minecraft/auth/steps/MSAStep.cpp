// SPDX-License-Identifier: GPL-3.0-only
/*
 *  SpikyMC - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "MSAStep.h"

#include <QAbstractOAuth2>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QOAuthHttpServerReplyHandler>
#include <QOAuthOobReplyHandler>

#include "Application.h"
#include "BuildConfig.h"
#include "FileSystem.h"

#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

bool isSchemeHandlerRegistered()
{
#ifdef Q_OS_LINUX
    QProcess process;
    process.start("xdg-mime", { "query", "default", "x-scheme-handler/" + BuildConfig.LAUNCHER_APP_BINARY_NAME });
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();

    return output.contains(APPLICATION->desktopFileName());

#elif defined(Q_OS_WIN)
    QString regPath = QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(BuildConfig.LAUNCHER_APP_BINARY_NAME);
    QSettings settings(regPath, QSettings::NativeFormat);

    const QString registeredRunCommand = settings.value("shell/open/command/.").toString().replace("\\", "/");
    return registeredRunCommand.contains(QCoreApplication::applicationFilePath());
#endif
    return true;
}

class CustomOAuthOobReplyHandler : public QOAuthOobReplyHandler {
    Q_OBJECT

   public:
    explicit CustomOAuthOobReplyHandler(QObject* parent = nullptr) : QOAuthOobReplyHandler(parent)
    {
        connect(APPLICATION, &Application::oauthReplyRecieved, this, &QOAuthOobReplyHandler::callbackReceived);
    }
    ~CustomOAuthOobReplyHandler() override
    {
        disconnect(APPLICATION, &Application::oauthReplyRecieved, this, &QOAuthOobReplyHandler::callbackReceived);
    }
    QString callback() const override { return BuildConfig.LAUNCHER_APP_BINARY_NAME + "://oauth/microsoft"; }

   protected:
    void networkReplyFinished(QNetworkReply* reply) override
    {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "OAuth2 request failed:" << reply->readAll();
        }

        QOAuthOobReplyHandler::networkReplyFinished(reply);
    }
};

class LoggingOAuthHttpServerReplyHandler final : public QOAuthHttpServerReplyHandler {
    Q_OBJECT

   public:
    explicit LoggingOAuthHttpServerReplyHandler(QObject* parent = nullptr) : QOAuthHttpServerReplyHandler(parent) {}
    explicit LoggingOAuthHttpServerReplyHandler(const QHostAddress& address, quint16 port, QObject* parent = nullptr)
        : QOAuthHttpServerReplyHandler(address, port, parent)
    {}

   protected:
    void networkReplyFinished(QNetworkReply* reply) override
    {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "OAuth2 request failed:" << reply->readAll();
        }

        QOAuthHttpServerReplyHandler::networkReplyFinished(reply);
    }
};

MSAStep::MSAStep(AccountData* data, bool silent) : AuthStep(data), m_silent(silent)
{
    m_clientId = APPLICATION->getMSAClientID();
    if (QCoreApplication::applicationFilePath().startsWith("/tmp/.mount_") || APPLICATION->isPortable() || !isSchemeHandlerRegistered())

    {
        auto replyHandler = new LoggingOAuthHttpServerReplyHandler(QHostAddress::Any, 0, this);
replyHandler->setCallbackText(
            tr("<html><head><meta charset=\"utf-8\"><title>SpikyMC</title></head>"
               "<body style=\"font-family: sans-serif; text-align: center; padding-top: 15vh; background: #2b2b2b; color: #fff;\">"
               "<svg width=\"512\" height=\"512\" viewBox=\"0 0 512 512\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">"
               "<rect width=\"512\" height=\"512\" fill=\"#52A535\"/>"
               "<path d=\"M225.617 210.102C225.617 244.399 197.721 272.203 163.309 272.203C128.896 272.203 101 244.399 101 210.102C101 175.804 128.896 148 163.309 148C197.721 148 225.617 175.804 225.617 210.102Z\" fill=\"#80D05D\"/>"
               "<path d=\"M410 210.102C410 244.399 382.104 272.203 347.691 272.203C313.279 272.203 285.383 244.399 285.383 210.102C285.383 175.804 313.279 148 347.691 148C382.104 148 410 175.804 410 210.102Z\" fill=\"#80D05D\"/>"
               "<path d=\"M167.124 326.701C167.124 356.484 193.827 376.762 224.982 381.832C237.062 385 259.315 386.641 286.654 381.832C308.272 378.029 343.877 358.385 343.877 326.701C343.877 299.402 317.809 267.768 286.654 277.273C255.5 286.778 255.5 286.778 224.982 277.273C194.463 267.768 164.58 298.818 167.124 326.701Z\" fill=\"#80D05D\"/>"
               "<path d=\"M225.617 195.102C225.617 229.399 197.721 257.203 163.309 257.203C128.896 257.203 101 229.399 101 195.102C101 160.804 128.896 133 163.309 133C197.721 133 225.617 160.804 225.617 195.102Z\" fill=\"#2A641C\"/>"
               "<path d=\"M410 195.102C410 229.399 382.104 257.203 347.691 257.203C313.279 257.203 285.383 229.399 285.383 195.102C285.383 160.804 313.279 133 347.691 133C382.104 133 410 160.804 410 195.102Z\" fill=\"#2A641C\"/>"
               "<path d=\"M167.124 311.701C167.124 341.484 193.827 361.762 224.982 366.832C237.062 370 259.315 371.641 286.654 366.832C308.272 363.029 343.877 343.385 343.877 311.701C343.877 284.402 317.809 252.768 286.654 262.273C255.5 271.778 255.5 271.778 224.982 262.273C194.463 252.768 164.58 283.818 167.124 311.701Z\" fill=\"#2A641C\"/>"
               "<path d=\"M225.617 200.102C225.617 234.399 197.721 262.203 163.309 262.203C128.896 262.203 101 234.399 101 200.102C101 165.804 128.896 138 163.309 138C197.721 138 225.617 165.804 225.617 200.102Z\" fill=\"black\"/>"
               "<path d=\"M410 200.102C410 234.399 382.104 262.203 347.691 262.203C313.279 262.203 285.383 234.399 285.383 200.102C285.383 165.804 313.279 138 347.691 138C382.104 138 410 165.804 410 200.102Z\" fill=\"black\"/>"
               "<path d=\"M167.124 316.701C167.124 346.484 193.827 366.762 224.982 371.832C237.062 375 259.315 376.641 286.654 371.832C308.272 368.029 343.877 348.385 343.877 316.701C343.877 289.402 317.809 257.768 286.654 267.273C255.5 276.778 255.5 276.778 224.982 267.273C194.463 257.768 164.58 288.818 167.124 316.701Z\" fill=\"black\"/>"
               "<h2>%1</h2><p>%2</p>"
               "<script>setTimeout(function () { window.close(); }, 1000);</script>"
               "</body></html>")
                .arg(tr("Sign in successful"), tr("You can close this window and return to the launcher.")));
        m_oauth2.setReplyHandler(replyHandler);
    } else {
        m_oauth2.setReplyHandler(new CustomOAuthOobReplyHandler(this));
    }
    m_oauth2.setAuthorizationUrl(QUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/authorize"));
    m_oauth2.setAccessTokenUrl(QUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token"));
    m_oauth2.setScope("XboxLive.SignIn XboxLive.offline_access");
    m_oauth2.setClientIdentifier(m_clientId);
    m_oauth2.setNetworkAccessManager(APPLICATION->network());

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, [this] {
        m_data->msaClientID = m_oauth2.clientIdentifier();
        m_data->msaToken.issueInstant = QDateTime::currentDateTimeUtc();
        m_data->msaToken.notAfter = m_oauth2.expirationAt();
        m_data->msaToken.extra = m_oauth2.extraTokens();
        m_data->msaToken.refresh_token = m_oauth2.refreshToken();
        m_data->msaToken.token = m_oauth2.token();
        emit finished(AccountTaskState::STATE_WORKING, tr("Got MSA token"));
    });
    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &MSAStep::authorizeWithBrowser);
    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, [this, silent](const QAbstractOAuth2::Error err) {
        auto state = AccountTaskState::STATE_FAILED_HARD;
        if (m_oauth2.status() == QAbstractOAuth::Status::Granted || silent) {
            if (err == QAbstractOAuth2::Error::NetworkError) {
                state = AccountTaskState::STATE_OFFLINE;
            } else {
                state = AccountTaskState::STATE_FAILED_SOFT;
            }
        }
        auto message = tr("Microsoft user authentication failed.");
        if (silent) {
            message = tr("Failed to refresh token.");
        }
        qWarning() << message;
        emit finished(state, message);
    });
    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::error, this,
            [this](const QString& error, const QString& errorDescription, const QUrl& uri) {
                qWarning() << "Failed to login because" << error << errorDescription;
                emit finished(AccountTaskState::STATE_FAILED_HARD, errorDescription);
            });

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::extraTokensChanged, this,
            [this](const QVariantMap& tokens) { m_data->msaToken.extra = tokens; });

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::clientIdentifierChanged, this,
            [this](const QString& clientIdentifier) { m_data->msaClientID = clientIdentifier; });
}

QString MSAStep::describe()
{
    return tr("Logging in with Microsoft account.");
}

void MSAStep::perform()
{
    if (m_silent) {
        if (m_data->msaClientID != m_clientId) {
            emit finished(AccountTaskState::STATE_DISABLED,
                          tr("Microsoft user authentication failed - client identification has changed."));
            return;
        }
        if (m_data->msaToken.refresh_token.isEmpty()) {
            emit finished(AccountTaskState::STATE_DISABLED, tr("Microsoft user authentication failed - refresh token is empty."));
            return;
        }
        m_oauth2.setRefreshToken(m_data->msaToken.refresh_token);
        m_oauth2.refreshAccessToken();
    } else {
        m_oauth2.setModifyParametersFunction(
            [](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* map) { map->insert("prompt", "select_account"); });

        *m_data = AccountData();
        m_data->msaClientID = m_clientId;
        m_oauth2.grant();
    }
}

#include "MSAStep.moc"
