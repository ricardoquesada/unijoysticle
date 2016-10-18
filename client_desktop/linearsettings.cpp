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

#include "linearsettings.h"
#include "ui_linearsettings.h"

#include "basejoymode.h"

LinearSettings::LinearSettings(BaseJoyMode *joyMode, QWidget *parent) :
    BaseSettings(joyMode, parent),
    ui(new Ui::LinearSettings)
{
    ui->setupUi(this);

    connect(ui->radioButton_joy1, &QRadioButton::clicked, [&]() {
        this->_joyMode->selectJoystick(1);
    });

    connect(ui->radioButton_joy2, &QRadioButton::clicked, [&]() {
        this->_joyMode->selectJoystick(2);
    });

    // default
    _joyMode->selectJoystick(2);
}

LinearSettings::~LinearSettings()
{
    delete ui;
}
