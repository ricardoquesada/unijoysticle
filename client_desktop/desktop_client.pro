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
    arrowswidget.cpp \
    linearform.cpp

HEADERS  += mainwindow.h \
    arrowswidget.h \
    linearform.h

FORMS    += mainwindow.ui \
    linearform.ui

RESOURCES += \
    resources.qrc
