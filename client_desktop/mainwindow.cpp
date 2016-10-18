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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_linearsettings.h"

#include <QMdiSubWindow>
#include <QHostInfo>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

#include "dpadsettings.h"
#include "linearsettings.h"
#include "commandowidget.h"
#include "dpadwidget.h"
#include "linearform.h"
#include "qMDNS.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _lastServerName(""),
    _settingsWidget(nullptr)
{
    ui->setupUi(this);
    QPixmap pixmap(1,1);
    pixmap.fill(QColor(0,0,0,0));
    QIcon icon(pixmap);

    auto dpadWidget = new DpadWidget(this);
    auto subWindowDpad = ui->mdiArea->addSubWindow(dpadWidget, Qt::Widget);
    subWindowDpad->setWindowTitle(tr("D-Pad mode"));
    subWindowDpad->setWindowIcon(icon);
    subWindowDpad->showMaximized();

    auto commandoWidget = new CommandoWidget(this);
    auto subWindow = ui->mdiArea->addSubWindow(commandoWidget, Qt::Widget);
    subWindow->setWindowTitle(tr("Commando mode"));
    subWindow->setWindowIcon(icon);
    subWindow->showMaximized();

    auto linearWidget = new LinearForm(this);
    subWindow = ui->mdiArea->addSubWindow(linearWidget, Qt::Widget);
    subWindow->setWindowTitle(tr("Linear mode"));
    subWindow->setWindowIcon(icon);
    subWindow->showMaximized();

    // enable tab #1
//    subWindowDpad->show();
//    ui->mdiArea->setActiveSubWindow(subWindowDpad);


    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);
    connect(qMDNS::getInstance(), &qMDNS::hostFound, this, &MainWindow::onDeviceDiscovered);
    connect(ui->lineEdit_server, &QLineEdit::editingFinished, this, &MainWindow::onResolveTriggered);
    connect(ui->pushButton_stats, &QPushButton::clicked, [&](){
        QDesktopServices::openUrl(QUrl(QString("http://") + ui->lineEdit_server->text()));
    });

    setUnifiedTitleAndToolBarOnMac(true);

    auto linearSettings = new LinearSettings(linearWidget, this);
    ui->verticalLayout_settings->insertWidget(1, linearSettings);
    _settingsWidget = linearSettings;

    onResolveTriggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSubWindowActivated(QMdiSubWindow* subwindow)
{
    // subwindow can be nullptr when closing the app
    if (subwindow)
    {
        delete _settingsWidget;
        _settingsWidget = nullptr;

        auto widget = static_cast<BaseJoyMode*>(subwindow->widget());

        if (dynamic_cast<DpadWidget*>(widget)) {
            auto settings = new DpadSettings(widget);
            ui->verticalLayout_settings->insertWidget(1, settings);
            _settingsWidget = settings;
        } else if (dynamic_cast<CommandoWidget*>(widget)) {
            /* no settings for Commando */
        } else if (dynamic_cast<LinearForm*>(widget)) {
            auto settings = new LinearSettings(widget);
            ui->verticalLayout_settings->insertWidget(1, settings);
            _settingsWidget = settings;
        }
    }
}

void MainWindow::onDeviceDiscovered (const QHostInfo& info)
{
    qDebug() << "";
    qDebug() << info.hostName().toStdString().c_str()
             << "has the following IPs:";

    if(info.addresses().length() > 0) {
        foreach (QHostAddress address, info.addresses())
            qDebug() << "  -" << address.toString();

        QHostAddress serverAddr(info.addresses().at(0));
        setServerAddress(serverAddr);
        setEnableTabs(true);
    }

    qDebug() << "";
}

void MainWindow::onResolveTriggered()
{
    QString newServerName = ui->lineEdit_server->text();

    if (_lastServerName != newServerName) {
        setEnableTabs(false);
        qDebug() << "Resolving: " << newServerName;
        qMDNS::getInstance()->lookup(newServerName);

        _lastServerName = newServerName;
    }
}

void MainWindow::setEnableTabs(bool enabled)
{
    for(auto& subwindow: ui->mdiArea->subWindowList()) {
        subwindow->setEnabled(enabled);
    }
    ui->mdiArea->setEnabled(enabled);
}

void MainWindow::setServerAddress(const QHostAddress& address)
{
    for(auto& subwindow: ui->mdiArea->subWindowList()) {
        BaseJoyMode *widget = static_cast<BaseJoyMode*>(subwindow->widget());
        widget->setServerAddress(address);
    }
}
