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

#ifndef ARROWSWIDGET_H
#define ARROWSWIDGET_H

#include <QWidget>
#include <QImage>

enum JoyBits {
    Up     = 0b00000001,
    Down   = 0b00000010,
    Left   = 0b00000100,
    Right  = 0b00001000,
    Fire   = 0b00010000,
    DPad   = 0b00001111,
    All    = 0b00011111,
};

class ArrowsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ArrowsWidget(QWidget *parent = 0);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    QImage _whiteImages[3];
    QImage _redImages[3];

    uint8_t _joyState;
};

#endif // ARROWSWIDGET_H
