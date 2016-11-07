#-------------------------------------------------
#
# Project created by QtCreator 2016-10-11T16:19:10
#
#-------------------------------------------------

QT       += core gui gamepad network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UniJoystiCle
target.path = $${PREFIX}/bin
INSTALLS += target
win32 {
    DESTDIR = ..
} else {
    DESTDIR = bin/
}

TEMPLATE = app

VERSION = 0.4.0
GIT_VERSION = $$system(git describe --abbrev=4 --dirty --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\" VERSION=\\\"$$VERSION\\\"

CONFIG += c++11
CONFIG += debug_and_release


SOURCES += main.cpp\
        mainwindow.cpp \
    commandowidget.cpp \
    dpadwidget.cpp \
    utils.cpp \
    qMDNS.cpp \
    basejoymode.cpp \
    basesettings.cpp \
    preferences.cpp \
    aboutdialog.cpp \
    commodorehomeform.cpp

HEADERS  += mainwindow.h \
    commandowidget.h \
    dpadwidget.h \
    utils.h \
    qMDNS.h \
    basejoymode.h \
    basesettings.h \
    preferences.h \
    aboutdialog.h \
    commodorehomeform.h

FORMS    += mainwindow.ui \
    aboutdialog.ui \
    commodorehomeform.ui

RESOURCES += \
    resources.qrc


# linux and mac
!win32 {
    QMAKE_CXXFLAGS += -Werror
}

win32 {
    RC_FILE = res/unijoysticle.rc
}
macx {
    TARGET = "UniJoystiCle Controller"
    ICON = res/unijoysticle-icon-mac.icns
    QMAKE_INFO_PLIST = res/Info.plist

    DISTFILES += \
        res/unijoysticle-icon-mac.icns
}
