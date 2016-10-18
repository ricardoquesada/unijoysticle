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

#include "basejoymode.h"

#include <QDebug>

static const quint16 SERVER_PORT = 6464;

BaseJoyMode::BaseJoyMode(QWidget *parent)
    : QWidget(parent)
{
    _socket = new QUdpSocket(this);
    _proto.version = 2;
    _proto.joyControl = 3;          // default, enable joy#1 and joy#2
}

void BaseJoyMode::setServerAddress(const QHostAddress& address)
{
    _host = address;
}

void BaseJoyMode::sendState()
{
    // send it two times
    for (int i=0; i<2; ++i)
        _socket->writeDatagram((char*)&_proto, sizeof(_proto), _host, SERVER_PORT);
}

void BaseJoyMode::selectJoystick(uint8_t joystick)
{
    _proto.joyControl = joystick;
}
