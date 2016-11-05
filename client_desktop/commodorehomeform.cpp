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

#include "commodorehomeform.h"
#include "ui_commodorehomeform.h"

#include <QSlider>

enum HomeCommands {
    NOTHING = 0,
    SONG_0 = 1,
    SONG_1 = 2,
    SONG_2 = 3,
    SONG_3 = 4,
    SONG_4 = 5,
    SONG_5 = 6,
    SONG_6 = 7,
    SONG_7 = 8,
    SONG_STOP = 9,
    SONG_PLAY = 10,
    SONG_PAUSE = 11,
    SONG_RESUME = 12,
    SONG_NEXT = 13,
    SONG_PREV = 14,
    DIMMER_0 = 15,
    DIMMER_25 = 16,
    DIMMER_50 = 17,
    DIMMER_75 = 18,
    DIMMER_100 = 19,
    ALARM_OFF = 20,
    ALARM_ON = 21,
    RESERVED0 = 22,
    RESERVED1 = 23,
    RESERVED2 = 24,
    RESERVED3 = 25,
    RESERVED4 = 26,
    RESERVED5 = 27,
    RESERVED6 = 28,
    RESERVED7 = 29,
    RESERVED8 = 30,
    RESERVED9 = 31,
};

CommodoreHomeForm::CommodoreHomeForm(QWidget *parent) :
    BaseJoyMode(parent),
    ui(new Ui::CommodoreHomeForm)
{
    ui->setupUi(this);

    _proto.version = 2;     // version 2
    _proto.joyControl = 3;  // joy 1 and 2
    _proto.joyStates[0] = 0;
    _proto.joyStates[1] = 0;

    QStringList list = {
        "Ashes to Ashes",
        "Final Countdown",
        "Pop Goes the World",
        "Jump",
        "Enola Gay",
        "Billie Jean",
        "Another Day in Paradise",
        "Wind of Change"
    };
    ui->comboBox_music->addItems(list);

    connect(ui->checkBox_alarm,&QCheckBox::stateChanged, [&](int status) {
        qDebug() << status;
        if (status == 0)
            _proto.joyStates[1] = ALARM_OFF;
        else
            _proto.joyStates[1] = ALARM_ON;
        sendState();
    });

    connect(ui->comboBox_music, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int index) {
        qDebug() << index;
        _proto.joyStates[1] = SONG_0 + (uint8_t)index;
        sendState();
    });

    connect(ui->horizontalSlider_dimmer , &QSlider::valueChanged, [&](int value) {
        value = (value / 25) * 25;
        qDebug() << value;
        ui->label_dimmer->setText(QString::number(value) + "%");
        ui->horizontalSlider_dimmer->setValue(value);
        _proto.joyStates[1] = DIMMER_0 + (uint8_t)value/25;
        sendState();
    });

//    connect(ui->horizontalSlider_joy2, &QSlider::valueChanged, [&](int value){
//        ui->label_joy2->setText(QString::number(value));
//        _proto.joyStates[1] = (uint8_t)value;
//        sendState();
//    });

}

CommodoreHomeForm::~CommodoreHomeForm()
{
    delete ui;
}
