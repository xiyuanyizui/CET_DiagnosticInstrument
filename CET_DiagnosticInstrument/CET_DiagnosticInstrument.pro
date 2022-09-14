#-------------------------------------------------
#
# Project created by QtCreator 2021-03-19T09:31:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CET_DiagnosticInstrument
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp
HEADERS += other/version.h
RC_FILE += other/cetapp.rc

INCLUDEPATH += \
	$$PWD/canthread \
	$$PWD/diagnose \
	$$PWD/filter \
	$$PWD/includes \
	$$PWD/mainwindow

include ($$PWD/canthread/canthread.pri)
include ($$PWD/diagnose/diagnose.pri)
include ($$PWD/filter/filter.pri)
include ($$PWD/includes/includes.pri)
include ($$PWD/mainwindow/mainwindow.pri)

win32: LIBS += -L$$PWD/lib -lControlCAN

