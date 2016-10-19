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

#include "linearform.h"
#include "ui_linearform.h"

#include <QSlider>


LinearForm::LinearForm(QWidget *parent) :
    BaseJoyMode(parent),
    ui(new Ui::LinearForm)
{
    ui->setupUi(this);

    _proto.joyControl = 3;  // joy 1 and 2

    connect(ui->horizontalSlider_joy1, &QSlider::valueChanged, [&](int value){
        ui->label_joy1->setText(QString::number(value));
        _proto.joyStates[0] = (uint8_t)value;
        sendState();
    });

    connect(ui->horizontalSlider_joy2, &QSlider::valueChanged, [&](int value){
        ui->label_joy2->setText(QString::number(value));
        _proto.joyStates[1] = (uint8_t)value;
        sendState();
    });

}

LinearForm::~LinearForm()
{
    delete ui;
}
