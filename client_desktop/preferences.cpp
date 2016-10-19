/****************************************************************************
Copyright 2016 Ricardo Quesada

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

#include "preferences.h"

#include <QColor>
#include <QDir>
#include <QDebug>

Preferences& Preferences::getInstance()
{
    static Preferences prefs;
    return prefs;
}

Preferences::Preferences()
    : _settings("RetroMoe","UniJoystiCle")
{
    qDebug() << _settings.fileName();
}

Preferences::~Preferences()
{
}

void Preferences::setCheckUpdates(bool enableIt)
{
    _settings.setValue(QLatin1String("Install/CheckForUpdates"), enableIt);
}

bool Preferences::getCheckUpdates() const
{
    return _settings.value(QLatin1String("Install/CheckForUpdates"), true).toBool();
}

void Preferences::setLastUpdateCheckDate(const QDateTime& date)
{
    _settings.setValue(QLatin1String("Install/LastUpdateCheckDate"), date);
}

QDateTime Preferences::getLastUpdateCheckDate() const
{
    return _settings.value(QLatin1String("Install/LastUpdateCheckDate"), QDateTime(QDate(2016,1,1))).toDateTime();
}

int Preferences::getLastTimeUpdateCheck() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastTime = getLastUpdateCheckDate();

    return lastTime.daysTo(now);
}

// default geometry / state
void Preferences::setMainWindowDefaultGeometry(const QByteArray& geometry)
{
    _settings.setValue(QLatin1String("MainWindow/defaultGeometry"), geometry);
}

QByteArray Preferences::getMainWindowDefaultGeometry() const
{
    return _settings.value(QLatin1String("MainWindow/defaultGeometry")).toByteArray();
}

void Preferences::setMainWindowDefaultState(const QByteArray& state)
{
    _settings.setValue(QLatin1String("MainWindow/defaultWindowState"), state);
}

QByteArray Preferences::getMainWindowDefaultState() const
{
    return _settings.value(QLatin1String("MainWindow/defaultWindowState")).toByteArray();
}

// user geometry / state

void Preferences::setMainWindowGeometry(const QByteArray& geometry)
{
    _settings.setValue(QLatin1String("MainWindow/geometry"), geometry);
}

QByteArray Preferences::getMainWindowGeometry() const
{
    return _settings.value(QLatin1String("MainWindow/geometry")).toByteArray();
}

void Preferences::setMainWindowState(const QByteArray& state)
{
    _settings.setValue(QLatin1String("MainWindow/windowState"), state);
}

QByteArray Preferences::getMainWindowState() const
{
    return _settings.value(QLatin1String("MainWindow/windowState")).toByteArray();
}

// server IP Address
void Preferences::setServerIPAddress(const QString& ipaddress)
{
    _settings.setValue(QLatin1String("server/ipaddress"), ipaddress);
}

QString Preferences::getServerIPAddress() const
{
    return _settings.value(QLatin1String("server/ipaddress"), "unijoysticle.local").toString();
}

// dpad
void Preferences::setDpadJumpWithB(bool enabled)
{
    _settings.setValue(QLatin1String("dpad/jumpWithB"), enabled);
}

bool Preferences::getDpadJumpWithB() const
{
    return _settings.value(QLatin1String("dpad/jumpWithB"), false).toBool();
}

void Preferences::setDpadSwapAB(bool enabled)
{
    _settings.setValue(QLatin1String("dpad/swapAB"), enabled);
}

bool Preferences::getDpadSwapAB() const
{
    return _settings.value(QLatin1String("dpad/swapAB"), false).toBool();
}

void Preferences::setDpadJoystick(int joy)
{
    _settings.setValue(QLatin1String("dpad/joystick"), joy);
}

int Preferences::getDpadJoystick() const
{
    return _settings.value(QLatin1String("dpad/joystick"), 2).toInt();
}
