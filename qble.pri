QT += dbus xml

HEADERS += \
    $$PWD/adaptermodel.h \
    $$PWD/qbleagent.h \
    $$PWD/qbledevice.h \
    $$PWD/qbleservice.h \
    $$PWD/qblecharacteristic.h \
    $$PWD/qbledescriptor.h \
    $$PWD/bluezadapter.h

SOURCES += \
    $$PWD/adaptermodel.cpp \
    $$PWD/qbleagent.cpp \
    $$PWD/qbledevice.cpp \
    $$PWD/qbleservice.cpp \
    $$PWD/qblecharacteristic.cpp \
    $$PWD/qbledescriptor.cpp \
    $$PWD/bluezadapter.cpp

