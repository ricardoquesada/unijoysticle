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

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QMdiSubWindow;
class QHostInfo;
class QHostAddress;
class QWidget;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onSubWindowActivated(QMdiSubWindow* subwindow);
    void onDeviceDiscovered (const QHostInfo& info);
    void onResolveTriggered ();

protected:
    void saveSettings();
    void showEvent(QShowEvent *ev);

private slots:
    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;

    void setEnableTabs(bool enabled);
    void setServerAddress(const QHostAddress& address);

    QString _lastServerName;

    QWidget *_settingsWidget;
};
