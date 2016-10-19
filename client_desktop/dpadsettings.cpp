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

#include "dpadsettings.h"
#include "ui_dpadsettings.h"

#include <QRadioButton>
#include <QCheckBox>

#include "basejoymode.h"
#include "dpadwidget.h"

DpadSettings::DpadSettings(BaseJoyMode* joyMode, QWidget *parent)
    : BaseSettings(joyMode, parent)
    , ui(new Ui::DpadSettings)
{
    ui->setupUi(this);

    connect(ui->radioButton_joy1, &QRadioButton::clicked, [&]() {
        this->_joyMode->selectJoystick(1);
    });

    connect(ui->radioButton_joy2, &QRadioButton::clicked, [&]() {
        this->_joyMode->selectJoystick(2);
    });

    connect(ui->checkBox_jumpB, &QCheckBox::clicked, [&](bool checked) {
        ui->checkBox_swapAB->setEnabled(checked);
        static_cast<DpadWidget*>(_joyMode)->setJumpWithB(checked);
    });

    connect(ui->checkBox_swapAB, &QCheckBox::clicked, [&](bool checked) {
        static_cast<DpadWidget*>(_joyMode)->setSwapAB(checked);
    });


    // default
    _joyMode->selectJoystick(2);
}

DpadSettings::~DpadSettings()
{
    delete ui;
}
