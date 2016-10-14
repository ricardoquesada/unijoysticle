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

#include "arrowswidget.h"

#include <QGamepad>
#include <QPainter>
#include <QPaintEvent>
#include <QTransform>
#include <QImage>

ArrowsWidget::ArrowsWidget(QWidget *parent) : QWidget(parent)
{

}

void ArrowsWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    // paint with default background color
    painter.fillRect(event->rect(), QWidget::palette().color(QWidget::backgroundRole()));

    QPen pen;
    pen.setColor(QWidget::palette().color(QWidget::backgroundRole()));
    pen.setWidth(1);
    pen.setStyle(Qt::PenStyle::SolidLine);
    painter.setPen(pen);

    QImage button(":/images/button.png");
    QImage arrow_top_right(":/images/arrow_bold_top_right.png");
    QImage arrow_right(":/images/arrow_bold_right.png");

    QSize imageSize = arrow_top_right.size();

    QPoint coords[9] = {
        QPoint(-1,-1), QPoint(0,-1), QPoint(1,-1),
        QPoint(-1,0), QPoint(0,0), QPoint(1,0),
        QPoint(-1,1), QPoint(0,1), QPoint(1,1)
    };
    int angles[9] = {
        -90, -90,   0,
        180,   0,   0,
        180,  90,  90,
    };
    int imagesToUse[9] = {
        0, 1, 0,
        1, 2, 1,
        0, 1, 0
    };
    QImage images[] = {arrow_top_right, arrow_right, button};

    for(int i=0; i<9; ++i) {
        int x = (coords[i].x() + 1) * imageSize.width();
        int y = (coords[i].y() + 1) * imageSize.height();

        QTransform rotating;
        rotating.rotate(angles[i]);
        QImage image = images[imagesToUse[i]].transformed(rotating);
        painter.drawImage(QPoint(x,y), image);
    }

    painter.end();
}

void ArrowsWidget::mousePressEvent(QMouseEvent *event)
{

}

void ArrowsWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void ArrowsWidget::keyPressEvent(QKeyEvent *event)
{

}

void ArrowsWidget::keyReleaseEvent(QKeyEvent *event)
{

}
