TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../eduipc/eduipc

QMAKE_LIBDIR += "$(TargetDir)"

QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_CXXFLAGS += /MP
QMAKE_LFLAGS_RELEASE += /DEBUG
QMAKE_LFLAGS += /MACHINE:X86
QMAKE_LFLAGS_DEBUG += /debugtype:cv,fixup
QMAKE_CXXFLAGS_WARN_ON += /W2
DEFINES += _CRT_SECURE_NO_DEPRECATE
DEFINES += _CRT_NONSTDC_NO_DEPRECATE

CharacterSet = 1
win32{
DEFINES += UNICODE
}

DEFINES += CC_IN_QT
DEFINES += _USE_NEW_CORE

//LIBS += eduipc.lib
//DEFINES += QYUTIL_DLL

CONFIG(debug, debug)
{
	DESTDIR = ../../../../shadow/bin/debug
}

CONFIG(release, release)
{
	DESTDIR = ../../../../shadow/bin/release
}

HEADERS +=  ipc-dll.h

SOURCES += 	main.cpp

