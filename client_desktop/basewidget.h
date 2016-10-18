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

#include <QWidget>
#include <QUdpSocket>

enum JoyBits {
    Up     = 0b00000001,
    Down   = 0b00000010,
    Left   = 0b00000100,
    Right  = 0b00001000,
    Fire   = 0b00010000,
    DPad   = 0b00001111,
    All    = 0b00011111,
};

// Protocol v2
#pragma pack(push, 0)
struct ProtoHeader {
    uint8_t version;        // should be 2
    uint8_t joyControl;     // 1, 2, or 3. which joysticks are enabled
    uint8_t joyStates[2];   // states for joy1 and joy2
};
#pragma pack(pop)
static_assert(sizeof(ProtoHeader) == 4, "Invalid size");

class BaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWidget(QWidget *parent = 0);
    void setServerAddress(const QHostAddress& address);
    void setEnabledJoysticks(uint8_t joyEnabled);

signals:

public slots:

protected:
    void sendState();

    QUdpSocket* _socket;
    QHostAddress _host;
    QByteArray _datagram;

    ProtoHeader _proto;
};
