#-------------------------------------------------
#
# Project created by QtCreator 2016-12-03T20:35:00
#
#-------------------------------------------------

QT       += core gui
QT       += serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CanSDOBlockServer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CanSDOBlockServer.c

HEADERS  += mainwindow.h \
    CanSDOBlockServer.h

FORMS    += mainwindow.ui

QMAKE_CFLAGS += -std=c11
