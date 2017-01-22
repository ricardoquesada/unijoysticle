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
#include <QTimer>

enum HomeCommands {
    NOTHING,
    SONG_0,
    SONG_1,
    SONG_2,
    SONG_3,
    SONG_4,
    SONG_5,
    SONG_6,
    SONG_7,
    SONG_8,
    RESERVED0,
    SONG_STOP,
    SONG_PLAY,
    SONG_PAUSE,
    SONG_RESUME,
    SONG_NEXT,
    SONG_PREV,
    DIMMER_0,
    DIMMER_25,
    DIMMER_50,
    DIMMER_75,
    DIMMER_100,
    ALARM_OFF,
    ALARM_ON,
    RESERVED1,
    RESERVED2,
    RESERVED3,
    RESERVED4,
    RESERVED5,
    RESERVED6,
    RESERVED7,
    RESERVED8
};

CommodoreHomeForm::CommodoreHomeForm(QWidget *parent) :
    BaseJoyMode(parent),
    ui(new Ui::CommodoreHomeForm)
{
    ui->setupUi(this);

    _timer = new QTimer(this);

    _proto.version = 2;     // version 2
    _proto.joyControl = 2;  // joy 2
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
        "Wind of Change",
        "Take my Breath Away"
    };
    ui->comboBox_music->addItems(list);

    connect(ui->checkBox_alarm,&QCheckBox::stateChanged, [&](int status) {
        qDebug() << status;
        if (status == 0)
            _proto.joyStates[1] = ALARM_OFF;
        else
            _proto.joyStates[1] = ALARM_ON;
        sendStateAndReset();
    });

    connect(ui->comboBox_music, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [&](int index) {
        qDebug() << index;
        _proto.joyStates[1] = SONG_0 + (uint8_t)index;
        sendStateAndReset();
    });

    connect(ui->horizontalSlider_dimmer , &QSlider::valueChanged, [&](int value) {
        value = (value / 25) * 25;
        qDebug() << value;
        ui->label_dimmer->setText(QString::number(value) + "%");
        ui->horizontalSlider_dimmer->setValue(value);
        _proto.joyStates[1] = DIMMER_0 + (uint8_t)value/25;
        sendStateAndReset();
    });

    connect(ui->pushButton_play, &QPushButton::clicked, [&]() {
        _proto.joyStates[1] = SONG_PLAY;
        sendStateAndReset();
    });

    connect(ui->pushButton_stop, &QPushButton::clicked, [&]() {
        _proto.joyStates[1] = SONG_STOP;
        sendStateAndReset();
    });

}

CommodoreHomeForm::~CommodoreHomeForm()
{
    delete ui;
}

void CommodoreHomeForm::enable(bool enabled)
{
    Q_UNUSED(enabled);
}

void CommodoreHomeForm::sendStateAndReset()
{
    sendState();
    _timer->singleShot(160, [&]() {
        _proto.joyStates[1] = 0;
        sendState();
    });
}
