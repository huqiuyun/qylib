#-------------------------------------------------
#
# Project created by QtCreator 2013-11-08T22:58:30
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += staticlib

TARGET = qytask
TEMPLATE = lib

CONFIG(debug, debug|release) {
     DESTDIR = ../../bin/debug
} else {
     DESTDIR = ../../bin/release
}

SOURCES += \
    qythread.cc \
    qythread_posix.cc \
    qytask.cc \
    qytaskrunner.cc \
    qymessagequeue.cc \
    qymessagethread.cc

HEADERS  += \
    qytask-config.h \
    qythread.h  \
    qythread_posix.h \
    qytask.h \
    qytaskrunner.h \
    qymessagequeue.h \
    qymessagethread.h \
    qymessagedata.h \
    qywake.h

INCLUDEPATH += ../include
INCLUDEPATH += /usr/include

OTHER_FILES +=
