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

#pragma once

#include "basejoymode.h"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace Ui {
class CommodoreHomeForm;
}

class CommodoreHomeForm : public BaseJoyMode
{
    Q_OBJECT

public:
    explicit CommodoreHomeForm(QWidget *parent = 0);
    ~CommodoreHomeForm();

    void enable(bool enabled);

private:
    void sendStateAndReset();
    Ui::CommodoreHomeForm *ui;
    QTimer* _timer;
};
