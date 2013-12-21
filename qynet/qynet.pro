#-------------------------------------------------
#
# Project created by QtCreator 2013-11-08T22:58:30
#
#-------------------------------------------------
CONFIG -= qt

TARGET = qynet
TEMPLATE = lib

CONFIG(debug, debug|release) {
     DESTDIR = ../../bin/debug
} else {
     DESTDIR = ../../bin/release
}

SOURCES += \
    qystream.cc \
    qystreamutils.cc \
    qystringutils.cc \
    qystringencode.cc \
    qyssladapter.cc \
    qysocketaddresspair.cc \
    qysocketaddress.cc \
    qysocketadapters.cc \
    qyopenssladapter.cc \
    qybytebuffer.cc \
    qyasyncudpsocket.cc \
    qyasynctcpsocket.cc \
    qyasyncpacketsocket.cc \
    qyphysicalsocketfactory.cc \
    qyphysicalsocket.cc

HEADERS  += \
    Equifax_Secure_Global_eBusiness_CA-1.h \
    qystream.h \
    qystreamutils.h \
    qystringutils.h \
    qystringencode.h \
    qyssladapter.h \
    qysocketfactory.h \
    qysocketaddress.h \
    qysocketadapters.h \
    qysocket.h \
    qyproxyinfo.h \
    qysocketaddresspair.h \
    qyopenssladapter.h \
    qybyteorder.h \
    qybytebuffer.h \
    qycriticalsection.h \
    qybasictypes.h \
    qyasyncudpsocket.h \
    qyasynctcpsocket.h \
    qyasyncsocket.h \
    qyasyncpacketsocket.h \
    qyphysicalsocketfactory.h \
    qyphysicalsocket.h

unix:CONFIG(release, debug|release): LIBS += -L/usr/lib -lssl -lcrypto
else:unix:CONFIG(debug, debug|release): LIBS += -L/usr/lib -lssl -lcrypto

INCLUDEPATH += ../include
