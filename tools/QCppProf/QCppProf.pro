#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T00:15:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QCppProf
TEMPLATE = app


SOURCES += main.cpp\
        qcppprofwindow.cpp \
    qcustomplot.cpp \
    threadlistentry.cpp

HEADERS  += qcppprofwindow.h \
    qcustomplot.h \
    threadlistentry.h

INCLUDEPATH += ../../src

QMAKE_CXXFLAGS+=-std=c++11
QMAKE_CC = gcc-4.8
QMAKE_CXX = g++-4.8

FORMS    += qcppprofwindow.ui \
    threadlistentry.ui
