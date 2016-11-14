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

#include "dpadwidget.h"

#include <QGamepad>
#include <QGamepadManager>
#include <QPainter>
#include <QPaintEvent>
#include <QTransform>
#include <QImage>
#include <QDebug>

#include "utils.h"
#include "preferences.h"

static const int ZOOM_LEVEL = 1;
static const float WIDTH = 480.0f;
static const float HEIGHT = 260.0f;

DpadWidget::DpadWidget(QWidget *parent)
    : BaseJoyMode(parent)
    , _joyState(0)
    , _processedJoyState(0)
    , _jumpWithB(false)
    , _swapAB(false)
    , _gamepadId(-1)
    , _gamepad(nullptr)
    , _zoomLevel(ZOOM_LEVEL)

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

    // default: joy 1
    _proto.joyControl = 1;

    setMinimumSize(QSize(WIDTH, HEIGHT));
}

void DpadWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto s = size();
    const auto tx = (s.width() - (WIDTH * _zoomLevel)) / 2.0f;
    const auto ty = (s.height() - (HEIGHT * _zoomLevel)) / 2.0;
    painter.translate(tx, ty);
    painter.scale(_zoomLevel, _zoomLevel);

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

    for(int i=0; i<9; ++i) {
        // 0.5 is the "anchor point". Use its center as the anchor point
        int x = (coords[i].x() - 0.5) * imageSize.width() + WIDTH / 2;
        int y = (coords[i].y() - 0.5) * imageSize.height() + HEIGHT / 2;

        QTransform rotating;
        rotating.rotate(angles[i]);
        QImage image;
        if (joyMask[i] & 0b00001111) {
            if (joyMask[i] == (_processedJoyState & 0b00001111))
                image = _redImages[imagesToUse[i]].transformed(rotating);
            else
                image = _whiteImages[imagesToUse[i]].transformed(rotating);
        } else {
            if (joyMask[i] == (_processedJoyState & 0b00010000))
                image = _redImages[imagesToUse[i]].transformed(rotating);
            else
                image = _whiteImages[imagesToUse[i]].transformed(rotating);
        }

        painter.drawImage(QPoint(x,y), image);
    }

    painter.end();
}

void DpadWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    // keep aspect ratio
    _zoomLevel = qMin(size().width() / WIDTH, size().height() / HEIGHT);
    update();
}

void DpadWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void DpadWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void DpadWidget::keyPressEvent(QKeyEvent *event)
{
    bool acceptEvent = false;
    switch (event->key()) {
    case Qt::Key_Left:
        _joyState |= JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_Right:
        _joyState |= JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_Down:
        _joyState |= JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_Up:
        _joyState |= JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_Z:
        // button A
        _joyState |= JoyBits::Fire;
        acceptEvent = true;
        break;
    case Qt::Key_X:
        // button B
        _joyState |= JoyBits::Button_B;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        processJoyState();
        repaint();
        send();
    }
    else
        QWidget::keyPressEvent(event);
}

void DpadWidget::keyReleaseEvent(QKeyEvent *event)
{
    bool acceptEvent = false;
    switch (event->key()) {
    case Qt::Key_Left:
        _joyState &= ~JoyBits::Left;
        acceptEvent = true;
        break;
    case Qt::Key_Right:
        _joyState &= ~JoyBits::Right;
        acceptEvent = true;
        break;
    case Qt::Key_Down:
        _joyState &= ~JoyBits::Down;
        acceptEvent = true;
        break;
    case Qt::Key_Up:
        _joyState &= ~JoyBits::Up;
        acceptEvent = true;
        break;
    case Qt::Key_Z:
        // Button A
        _joyState &= ~JoyBits::Fire;
        acceptEvent = true;
        break;
    case Qt::Key_X:
        // Button B
        _joyState &= ~JoyBits::Button_B;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        processEvents();
    }
    else
        QWidget::keyPressEvent(event);
}

void DpadWidget::processJoyState()
{
    _processedJoyState = _joyState;

    if (_jumpWithB) {
        if (_swapAB) {
            // swap bit 4 and 5
            uint8_t ab = (_processedJoyState & 0b00010000) << 1;  // button A -> B
            ab |=        (_processedJoyState & 0b00100000) >> 1;  // button B -> A

            _processedJoyState &= 0b11001111;
            _processedJoyState |= ab;
        }
        // turn of jump
        _processedJoyState &= 0b11111110;

        // get "b" value and put it in bit 0
        uint8_t jump_with_b= (_processedJoyState & 0b00100000) >> 5;
        _processedJoyState |= jump_with_b;
        _processedJoyState &= 0b11011111;         // turn off "button b" bit
    }
}

void DpadWidget::setJumpWithB(bool enabled)
{
    _jumpWithB = enabled;
}

void DpadWidget::setSwapAB(bool enabled)
{
    _swapAB = enabled;
}


void DpadWidget::send()
{
    // copy local state to "protocol"
    _proto.joyStates[_proto.joyControl-1] = _processedJoyState;

    // send it
    sendState();
}

void DpadWidget::processEvents()
{
    processJoyState();
    repaint();
    send();
}


// slots
void DpadWidget::onJoy1Clicked()
{
    _proto.joyControl = 1;
}

void DpadWidget::onJoy2Clicked()
{
    _proto.joyControl = 2;
}

void DpadWidget::onJumpBChecked(bool checked)
{
    _jumpWithB = checked;
}

void DpadWidget::onSwapABChecked(bool checked)
{
    _swapAB = checked;
}

void DpadWidget::onGamepadConnected(int id)
{
    if (!_gamepad) {
        _gamepadId = id;
        _gamepad = new QGamepad(id, this);

        connect(_gamepad, &QGamepad::axisLeftXChanged, this, &DpadWidget::onAxisLeftXChanged);
        connect(_gamepad, &QGamepad::axisLeftYChanged, this, &DpadWidget::onAxisLeftYChanged);
        connect(_gamepad, &QGamepad::buttonAChanged, this, &DpadWidget::onButtonAChanged);
        connect(_gamepad, &QGamepad::buttonBChanged, this, &DpadWidget::onButtonBChanged);
        connect(_gamepad, &QGamepad::buttonUpChanged, this, &DpadWidget::onButtonUpChanged);
        connect(_gamepad, &QGamepad::buttonDownChanged, this, &DpadWidget::onButtonDownChanged);
        connect(_gamepad, &QGamepad::buttonLeftChanged, this, &DpadWidget::onButtonLeftChanged);
        connect(_gamepad, &QGamepad::buttonRightChanged, this, &DpadWidget::onButtonRightChanged);
    }
}

void DpadWidget::onGamepadDisconnected(int id)
{
    if (_gamepadId == id) {
        unregisterGamepad();
    }
}

void DpadWidget::enable(bool enabled)
{
    if (enabled)
        registerGamepad();
    else
        unregisterGamepad();
}

void DpadWidget::registerGamepad()
{
    if (!_gamepad) {
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this, &DpadWidget::onGamepadConnected);
        connect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this, &DpadWidget::onGamepadDisconnected);

        auto connectedDevices = QGamepadManager::instance()->connectedGamepads();
        if (connectedDevices.length() > 0)
            onGamepadConnected(connectedDevices.at(0));
    }
}

void DpadWidget::unregisterGamepad()
{
    if (_gamepad) {
        disconnect(_gamepad, &QGamepad::axisLeftXChanged, this, &DpadWidget::onAxisLeftXChanged);
        disconnect(_gamepad, &QGamepad::axisLeftYChanged, this, &DpadWidget::onAxisLeftYChanged);
        disconnect(_gamepad, &QGamepad::buttonAChanged, this, &DpadWidget::onButtonAChanged);
        disconnect(_gamepad, &QGamepad::buttonBChanged, this, &DpadWidget::onButtonBChanged);
        disconnect(_gamepad, &QGamepad::buttonUpChanged, this, &DpadWidget::onButtonUpChanged);
        disconnect(_gamepad, &QGamepad::buttonDownChanged, this, &DpadWidget::onButtonDownChanged);
        disconnect(_gamepad, &QGamepad::buttonLeftChanged, this, &DpadWidget::onButtonLeftChanged);
        disconnect(_gamepad, &QGamepad::buttonRightChanged, this, &DpadWidget::onButtonRightChanged);

        disconnect(QGamepadManager::instance(), &QGamepadManager::gamepadConnected, this, &DpadWidget::onGamepadConnected);
        disconnect(QGamepadManager::instance(), &QGamepadManager::gamepadDisconnected, this, &DpadWidget::onGamepadDisconnected);

        delete _gamepad;
        _gamepad = nullptr;
        _gamepadId = -1;
    }
}

void DpadWidget::onAxisLeftXChanged(double value)
{
    qDebug() << "DpadWidget Left X" << value;
    bool changed = false;
    if (value > 0.8) {
        this->_joyState |= JoyBits::Right;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState |= JoyBits::Left;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState &= ~JoyBits::Right;
        this->_joyState &= ~JoyBits::Left;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void DpadWidget::onAxisLeftYChanged(double value)
{
    qDebug() << "Left Y" << value;
    bool changed = false;

    if (value > 0.8) {
        this->_joyState |= JoyBits::Down;
        changed = true;
    } else if (value < -0.8) {
        this->_joyState |= JoyBits::Up;
        changed = true;
    }
    else if (value > -0.1 && value < 0.1) {
        this->_joyState &= ~JoyBits::Up;
        this->_joyState &= ~JoyBits::Down;
        changed = true;
    }
    if (changed) {
        processEvents();
    }
}

void DpadWidget::onButtonAChanged(bool pressed)
{
    qDebug() << "Button A" << pressed;
    if (pressed)
        _joyState |= JoyBits::Fire;
    else
        _joyState &= ~JoyBits::Fire;
    processEvents();
}

void DpadWidget::onButtonBChanged(bool pressed)
{
    qDebug() << "Button B" << pressed;
    if (pressed)
        _joyState |= JoyBits::Button_B;
    else
        _joyState &= ~JoyBits::Button_B;
    processEvents();
}

void DpadWidget::onButtonUpChanged(bool pressed)
{
    qDebug() << "Button Up" << pressed;
    if (pressed)
        _joyState |= JoyBits::Up;
    else
        _joyState &= ~JoyBits::Up;
    processEvents();
}

void DpadWidget::onButtonDownChanged(bool pressed)
{
    qDebug() << "Button Down" << pressed;
    if (pressed)
        _joyState |= JoyBits::Down;
    else
        _joyState &= ~JoyBits::Down;
    processEvents();
}

void DpadWidget::onButtonLeftChanged(bool pressed)
{
    qDebug() << "Button Left" << pressed;
    if (pressed)
        _joyState |= JoyBits::Left;
    else
        _joyState &= ~JoyBits::Left;
    processEvents();
}

void DpadWidget::onButtonRightChanged(bool pressed)
{
    qDebug() << "Button Right" << pressed;
    if (pressed)
        _joyState |= JoyBits::Right;
    else
        _joyState &= ~JoyBits::Right;
    processEvents();
}
