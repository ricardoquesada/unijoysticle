/****************************************************************************
Copyright 2016 Ricardo Quesada
http://github.com/ricardoquesada/unijoysticle

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#pragma once

#include <QObject>
#include <QDateTime>
#include <QSettings>

class Preferences: public QObject
{
    Q_OBJECT

public:
    static Preferences& getInstance();

    Preferences(Preferences const&)     = delete;
    void operator=(Preferences const&)  = delete;

    void setCheckUpdates(bool enableIt);
    bool getCheckUpdates() const;

    void setLastUpdateCheckDate(const QDateTime& date);
    QDateTime getLastUpdateCheckDate() const;
    int getLastTimeUpdateCheck() const;

    void setMainWindowDefaultGeometry(const QByteArray &geometry);
    QByteArray getMainWindowDefaultGeometry() const;
    void setMainWindowDefaultState(const QByteArray &state);
    QByteArray getMainWindowDefaultState() const;

    void setMainWindowGeometry(const QByteArray &geometry);
    QByteArray getMainWindowGeometry() const;
    void setMainWindowState(const QByteArray &state);
    QByteArray getMainWindowState() const;

    void setServerIPAddress(const QString& ipaddress);
    QString getServerIPAddress() const;

    void setDpadJumpWithB(bool enabled);
    bool getDpadJumpWithB() const;

    void setDpadSwapAB(bool enabled);
    bool getDpadSwapAB() const;

    void setDpadJoystick(int joy);
    int getDpadJoystick() const;

private:
    Preferences();
    ~Preferences();

    QSettings _settings;
};
