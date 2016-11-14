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

#include "commandowidget.h"

#include <QGamepad>
#include <QGamepadManager>
#include <QPainter>
#include <QPaintEvent>
#include <QTransform>
#include <QImage>
#include <QDebug>

#include "utils.h"


CommandoWidget::CommandoWidget(QWidget *parent)
    : BaseJoyMode(parent)
    , _gamepadId(-1)
    , _gamepad(nullptr)
{
    QImage button(":/images/button.png");
    QImage arrow_top_right(":/images/arrow_bold_top_right.png");
    QImage arrow_right(":/images/arrow_bold_right.png");

    _whiteImages[0] = arrow_top_right;
    _whiteImages[1] = arrow_right;
    _whiteImages[2] = button;

    _redImages[0] = utils_tinted(arrow_top_right, QColor(255,0,0), QPainter::CompositionMode_Source);
    _redImages[1] = utils_tinted(arrow_right, QColor(255,0,0), QPainter::CompositionMode_Source);
    _redImages[2] = utils_tinted(button, QColor(255,0,0), QPainter::CompositionMode_Source);

    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setFocus();

    // send two joysticks
    _proto.version = 2;
    _proto.joyControl = 3;
    _joyState[0] = 0;
    _joyState[1] = 0;
}

void CommandoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);


    // paint with default background color
    painter.fillRect(event->rect(), QWidget::palette().color(QWidget::backgroundRole()));

    QPen pen;
    pen.setColor(QWidget::palette().color(QWidget::backgroundRole()));
    pen.setWidth(1);
    pen.setStyle(Qt::PenStyle::SolidLine);
    painter.setPen(pen);


    QSize imageSize = _whiteImages[0].size();

    const QPoint coords[9] = {
        QPoint(-1,-1), QPoint(0,-1), QPoint(1,-1),
        QPoint(-1,0), QPoint(0,0), QPoint(1,0),
        QPoint(-1,1), QPoint(0,1), QPoint(1,1)
    };
    const int angles[9] = {
        -90, -90,   0,
        180,   0,   0,
        180,  90,  90,
    };
    const int imagesToUse[9] = {
        0, 1, 0,
        1, 2, 1,
        0, 1, 0
    };
    const uint8_t joyMask[9] = {
        0b0101, /* top-left */
        0b0001, /* top */
        0b1001, /* top-right */
        0b0100, /* left */
        0b10000, /* fire */
        0b1000, /* right */
        0b0110, /* bottom-left */
        0b0010, /* bottom */
        0b1010, /* bottom-right */
    };

    auto s = size();

    for (int j=0; j<2; ++j) {
        for(int i=0; i<9; ++i) {
            // 0.5 is the "anchor point". Use its center as the anchor point
            int x = (coords[i].x() - 0.5) * imageSize.width() + (s.width() / 4) * (j*2+1);
            int y = (coords[i].y() - 0.5) * imageSize.height() + s.height() / 2;

            QTransform rotating;
            rotating.rotate(angles[i]);
            QImage image;
            if (joyMask[i] & 0b00001111) {
                if (joyMask[i] == (_joyState[j] & 0b00001111))
                    image = _redImages[imagesToUse[i]].transformed(rotating);
                else
                    image = _whiteImages[imagesToUse[i]].transformed(rotating);
            } else {
                if (joyMask[i] == (_joyState[j] & 0b00010000))
                    image = _redImages[imagesToUse[i]].transformed(rotating);
                else
                    image = _whiteImages[imagesToUse[i]].transformed(rotating);
            }

            painter.drawImage(QPoint(x,y), image);
        }
    }

    painter.end();
}

void CommandoWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void CommandoWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void CommandoWidget::keyPressEvent(QKeyEvent *event)
{
    bool acceptEvent = false;
    switch (event->key()) {

    // Joy #2
    case Qt::Key_Left:
    case Qt::Key_J:
        _joyState[1] |= JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_L:
        _joyState[1] |= JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_Down:
    case Qt::Key_K:
        _joyState[1] |= JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_Up:
    case Qt::Key_I:
        _joyState[1] |= JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_Z:
    case Qt::Key_O:
        _joyState[1] |= JoyBits::Fire;
        acceptEvent = true;
        break;

    // Joy #1
    case Qt::Key_A:
        _joyState[0] |= JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_D:
        _joyState[0] |= JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_S:
        _joyState[0] |= JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_W:
        _joyState[0] |= JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_E:
    case Qt::Key_X:
        _joyState[0] |= JoyBits::Fire;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        repaint();
        send();
    }
    else
        QWidget::keyPressEvent(event);
}

void CommandoWidget::keyReleaseEvent(QKeyEvent *event)
{
    bool acceptEvent = false;
    switch (event->key()) {
    // Joy #2
    case Qt::Key_Left:
    case Qt::Key_J:
        _joyState[1] &= ~JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_L:
        _joyState[1] &= ~JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_Down:
    case Qt::Key_K:
        _joyState[1] &= ~JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_Up:
    case Qt::Key_I:
        _joyState[1] &= ~JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_Z:
    case Qt::Key_O:
        _joyState[1] &= ~JoyBits::Fire;
        acceptEvent = true;
        break;

    // Joy #1
    case Qt::Key_A:
        _joyState[0] &= ~JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_D:
        _joyState[0] &= ~JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_S:
        _joyState[0] &= ~JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_W:
        _joyState[0] &= ~JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_E:
    case Qt::Key_X:
        _joyState[0] &= ~JoyBits::Fire;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        repaint();
        send();
    }
    else
        QWidget::keyPressEvent(event);
}

void CommandoWidget::processEvents()
{
    repaint();
    send();
}

void CommandoWidget::send()
{
    _proto.joyStates[0] = _joyState[0];
    _proto.joyStates[1] = _joyState[1];
    sendState();
}

void CommandoWidget::onGamepadConnected(int id)
{
    if (!_gamepad) {
        _gamepadId = id;
        _gamepad = new QGamepad(id, this);

        connect(_gamepad, &QGamepad::axisLeftXChanged, this, &CommandoWidget::onAxisLeftXChanged);
        connect(_gamepad, &QGamepad::axisLeftYChanged, this, &CommandoWidget::onAxisLeftYChanged);
        connect(_gamepad, &QGamepad::axisRightXChanged, this, &CommandoWidget::onAxisRightXChanged);
        connect(_gamepad, &QGamepad::axisRightYChanged, this, &CommandoWidget::onAxisRightYChanged);
        connect(_gamepad, &QGamepad::buttonAChanged, this, &CommandoWidget::onButtonAChanged);
        connect(_gamepad, &QGamepad::buttonBChanged, this, &CommandoWidget::onButtonBChanged);
        connect(_gamepad, &QGamepad::buttonXChanged, this, &CommandoWidget::onButtonXChanged);
        connect(_gamepad, &QGamepad::buttonYChanged, this, &CommandoWidget::onButtonYChanged);
        connect(_gamepad, &QGamepad::buttonUpChanged, this, &CommandoWidget::onButtonUpChanged);
        connect(_gamepad, &QGamepad::buttonDownChanged, this, &CommandoWidget::onButtonDownChanged);
        connect(_gamepad, &QGamepad::buttonLeftChanged, this, &CommandoWidget::onButtonLeftChanged);
        connect(_gamepad, &QGamepad::buttonRightChanged, this, &CommandoWidget::onButtonRightChanged);
    }
}

void CommandoWidget::onGamepadDisconnected(int id)
{
    if (_gamepadId == id) {
        unregisterGamepad();
    }
}

void CommandoWidget::enable(bool enabled)
{
    if (enabled)
        registerGamepad();
    else
        unregisterGamepad();
}

void CommandoWidget::registerGamepad()
{
    if (!_gamepad) {
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this, &CommandoWidget::onGamepadConnected);
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this, &CommandoWidget::onGamepadDisconnected);

        auto connectedDevices = QGamepadManager::instance()->connectedGamepads();
        if (connectedDevices.length() > 0)
            onGamepadConnected(connectedDevices.at(0));
    }
}

void CommandoWidget::unregisterGamepad()
{
    if (_gamepad) {
        disconnect(_gamepad, &QGamepad::axisLeftXChanged, this, &CommandoWidget::onAxisLeftXChanged);
        disconnect(_gamepad, &QGamepad::axisLeftYChanged, this, &CommandoWidget::onAxisLeftYChanged);
        disconnect(_gamepad, &QGamepad::axisRightXChanged, this, &CommandoWidget::onAxisRightXChanged);
        disconnect(_gamepad, &QGamepad::axisRightYChanged, this, &CommandoWidget::onAxisRightYChanged);
        disconnect(_gamepad, &QGamepad::buttonAChanged, this, &CommandoWidget::onButtonAChanged);
        disconnect(_gamepad, &QGamepad::buttonBChanged, this, &CommandoWidget::onButtonBChanged);
        disconnect(_gamepad, &QGamepad::buttonXChanged, this, &CommandoWidget::onButtonXChanged);
        disconnect(_gamepad, &QGamepad::buttonYChanged, this, &CommandoWidget::onButtonYChanged);
        disconnect(_gamepad, &QGamepad::buttonUpChanged, this, &CommandoWidget::onButtonUpChanged);
        disconnect(_gamepad, &QGamepad::buttonDownChanged, this, &CommandoWidget::onButtonDownChanged);
        disconnect(_gamepad, &QGamepad::buttonLeftChanged, this, &CommandoWidget::onButtonLeftChanged);
        disconnect(_gamepad, &QGamepad::buttonRightChanged, this, &CommandoWidget::onButtonRightChanged);

        disconnect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this, &CommandoWidget::onGamepadConnected);
        disconnect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this, &CommandoWidget::onGamepadDisconnected);

        delete _gamepad;
        _gamepad = nullptr;
        _gamepadId = -1;
    }
}

void CommandoWidget::onAxisLeftXChanged(double value)
{
    qDebug() << "CommandoWidget Left X" << value;
    bool changed = false;
    if (value > 0.8) {
        this->_joyState[1] |= JoyBits::Right;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState[1] |= JoyBits::Left;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState[1] &= ~JoyBits::Right;
        this->_joyState[1] &= ~JoyBits::Left;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void CommandoWidget::onAxisLeftYChanged(double value)
{
    qDebug() << "Left Y" << value;
    bool changed = false;

    if (value > 0.8) {
        this->_joyState[1] |= JoyBits::Down;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState[1] |= JoyBits::Up;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState[1] &= ~JoyBits::Up;
        this->_joyState[1] &= ~JoyBits::Down;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void CommandoWidget::onAxisRightXChanged(double value)
{
    qDebug() << "CommandoWidget Left X" << value;
    bool changed = false;
    if (value > 0.8) {
        this->_joyState[0] |= JoyBits::Right;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState[0] |= JoyBits::Left;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState[0] &= ~JoyBits::Right;
        this->_joyState[0] &= ~JoyBits::Left;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void CommandoWidget::onAxisRightYChanged(double value)
{
    qDebug() << "Left Y" << value;
    bool changed = false;

    if (value > 0.8) {
        this->_joyState[0] |= JoyBits::Down;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState[0] |= JoyBits::Up;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState[0] &= ~JoyBits::Up;
        this->_joyState[0] &= ~JoyBits::Down;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void CommandoWidget::onButtonAChanged(bool pressed)
{
    qDebug() << "Button A" << pressed;
    if (pressed)
        _joyState[1] |= JoyBits::Fire;
    else
        _joyState[1] &= ~JoyBits::Fire;
    processEvents();
}

void CommandoWidget::onButtonBChanged(bool pressed)
{
    qDebug() << "Button B" << pressed;
    if (pressed)
        _joyState[0] |= JoyBits::Fire;
    else
        _joyState[0] &= ~JoyBits::Fire;
    processEvents();
}

void CommandoWidget::onButtonXChanged(bool pressed)
{
    qDebug() << "Button B" << pressed;
    if (pressed)
        _joyState[0] |= JoyBits::Down;
    else
        _joyState[0] &= ~JoyBits::Down;
    processEvents();
}

void CommandoWidget::onButtonYChanged(bool pressed)
{
    qDebug() << "Button B" << pressed;
    if (pressed)
        _joyState[0] |= JoyBits::Right;
    else
        _joyState[0] &= ~JoyBits::Right;
    processEvents();
}


void CommandoWidget::onButtonUpChanged(bool pressed)
{
    qDebug() << "Button Up" << pressed;
    if (pressed)
        _joyState[1] |= JoyBits::Up;
    else
        _joyState[1] &= ~JoyBits::Up;
    processEvents();
}

void CommandoWidget::onButtonDownChanged(bool pressed)
{
    qDebug() << "Button Down" << pressed;
    if (pressed)
        _joyState[1] |= JoyBits::Down;
    else
        _joyState[1] &= ~JoyBits::Down;
    processEvents();
}

void CommandoWidget::onButtonLeftChanged(bool pressed)
{
    qDebug() << "Button Left" << pressed;
    if (pressed)
        _joyState[1] |= JoyBits::Left;
    else
        _joyState[1] &= ~JoyBits::Left;
    processEvents();
}

void CommandoWidget::onButtonRightChanged(bool pressed)
{
    qDebug() << "Button Right" << pressed;
    if (pressed)
        _joyState[1] |= JoyBits::Right;
    else
        _joyState[1] &= ~JoyBits::Right;
    processEvents();
}
