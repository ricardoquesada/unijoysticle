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
#include <QCloseEvent>

#include "preferences.h"
#include "dpadsettings.h"
#include "commandowidget.h"
#include "dpadwidget.h"
#include "linearform.h"
#include "qMDNS.h"

static const int STATE_VERSION = 1;

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

    auto dpadWidget = new DpadWidget;
    auto subWindowDpad = ui->mdiArea->addSubWindow(dpadWidget, Qt::Widget);
    subWindowDpad->setWindowTitle(tr("D-Pad mode"));
    subWindowDpad->setWindowIcon(icon);
    subWindowDpad->showMaximized();
    subWindowDpad->layout()->setContentsMargins(2, 2, 2, 2);

    auto commandoWidget = new CommandoWidget;
    auto subWindow = ui->mdiArea->addSubWindow(commandoWidget, Qt::Widget);
    subWindow->setWindowTitle(tr("Commando mode"));
    subWindow->setWindowIcon(icon);
    subWindow->showMaximized();
    subWindow->layout()->setContentsMargins(2, 2, 2, 2);

    auto linearWidget = new LinearForm;
    subWindow = ui->mdiArea->addSubWindow(linearWidget, Qt::Widget);
    subWindow->setWindowTitle(tr("Linear mode"));
    subWindow->setWindowIcon(icon);
    subWindow->showMaximized();
    subWindow->layout()->setContentsMargins(2, 2, 2, 2);

    // enable tab #1
//    ui->mdiArea->setActiveSubWindow(subWindowDpad);
//    ui->mdiArea->setFocus();
//    subWindowDpad->show();
//    subWindowDpad->setFocus();


    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);
    connect(qMDNS::getInstance(), &qMDNS::hostFound, this, &MainWindow::onDeviceDiscovered);
    connect(ui->lineEdit_server, &QLineEdit::editingFinished, this, &MainWindow::onResolveTriggered);
    connect(ui->pushButton_stats, &QPushButton::clicked, [&](){
        QDesktopServices::openUrl(QUrl(QString("http://") + ui->lineEdit_server->text()));
    });

    setUnifiedTitleAndToolBarOnMac(true);

    restoreSettings();
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
            /* no settings for Linear */
        }
    }
}

void MainWindow::onDeviceDiscovered (const QHostInfo& info)
{
    qDebug() << "";
    qDebug() << info.hostName()
             << "has the following IPs:";
    foreach (QHostAddress address, info.addresses())
        qDebug() << "  -" << address.toString();
    qDebug() << "";

    if(info.hostName() == ui->lineEdit_server->text() && info.addresses().length() > 0) {
        QHostAddress serverAddr(info.addresses().at(0));
        setServerAddress(serverAddr);
        setEnableTabs(true);
    }
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
    ui->pushButton_stats->setEnabled(enabled);
    if (_settingsWidget)
        _settingsWidget->setEnabled(enabled);
}

void MainWindow::setServerAddress(const QHostAddress& address)
{
    for(auto& subwindow: ui->mdiArea->subWindowList()) {
        BaseJoyMode *widget = static_cast<BaseJoyMode*>(subwindow->widget());
        widget->setServerAddress(address);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    saveSettings();
    QApplication::exit();
}

void MainWindow::restoreSettings()
{
    // before restoring settings, save the current layout
    // needed for "reset layout"
    auto& preferences = Preferences::getInstance();
    preferences.setMainWindowDefaultGeometry(saveGeometry());
    preferences.setMainWindowDefaultState(saveState(STATE_VERSION));

    auto geom = preferences.getMainWindowGeometry();
    auto state = preferences.getMainWindowState();

    restoreState(state, STATE_VERSION);
    restoreGeometry(geom);

    auto serverAddress = preferences.getServerIPAddress();
    ui->lineEdit_server->setText(serverAddress);
}

void MainWindow::saveSettings()
{
    auto& preferences = Preferences::getInstance();
    preferences.setMainWindowGeometry(saveGeometry());
    preferences.setMainWindowState(saveState(STATE_VERSION));

    auto serverAddress = ui->lineEdit_server->text();
    preferences.setServerIPAddress(serverAddress);
}

void MainWindow::showEvent( QShowEvent* event ) {
    QMainWindow::showEvent( event );

    // execute this after the main window has been created
    onResolveTriggered();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    saveSettings();
}
