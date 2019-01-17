#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T00:15:02
#
#-------------------------------------------------

QT       += core gui printsupport

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

FORMS    += qcppprofwindow.ui \
    threadlistentry.ui
