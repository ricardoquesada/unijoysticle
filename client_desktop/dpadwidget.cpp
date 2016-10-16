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

DpadWidget::DpadWidget(QWidget *parent)
    : BaseWidget(parent)
    , _joyState(0)
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

    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, [&](){

        qDebug() << QGamepadManager::instance()->connectedGamepads();
    });

    qDebug() << "Connected gamepads:";
    qDebug() << QGamepadManager::instance()->connectedGamepads();

    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setFocus();
}

void DpadWidget::paintEvent(QPaintEvent *event)
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

    for(int i=0; i<9; ++i) {
        // 0.5 is the "anchor point". Use its center as the anchor point
        int x = (coords[i].x() - 0.5) * imageSize.width() + s.width() / 2;
        int y = (coords[i].y() - 0.5) * imageSize.height() + s.height() / 2;

        QTransform rotating;
        rotating.rotate(angles[i]);
        QImage image;
        if (joyMask[i] & 0b00001111) {
            if (joyMask[i] == (_joyState & 0b00001111))
                image = _redImages[imagesToUse[i]].transformed(rotating);
            else
                image = _whiteImages[imagesToUse[i]].transformed(rotating);
        } else {
            if (joyMask[i] == (_joyState & 0b00010000))
                image = _redImages[imagesToUse[i]].transformed(rotating);
            else
                image = _whiteImages[imagesToUse[i]].transformed(rotating);
        }

        painter.drawImage(QPoint(x,y), image);
    }

    painter.end();
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
    case Qt::Key_X:
        _joyState |= JoyBits::Fire;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        repaint();
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
    case Qt::Key_X:
        _joyState &= ~JoyBits::Fire;
        acceptEvent = true;
        break;
    }
    if (acceptEvent) {
        event->accept();
        repaint();
    }
    else
        QWidget::keyPressEvent(event);
}
