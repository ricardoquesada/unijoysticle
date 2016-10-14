#-------------------------------------------------
#
# Project created by QtCreator 2016-10-11T16:19:10
#
#-------------------------------------------------

QT       += core gui gamepad

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UniJoystiCle
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    linearform.cpp \
    commandowidget.cpp \
    dpadwidget.cpp

HEADERS  += mainwindow.h \
    linearform.h \
    commandowidget.h \
    dpadwidget.h

FORMS    += mainwindow.ui \
    linearform.ui

RESOURCES += \
    resources.qrc
