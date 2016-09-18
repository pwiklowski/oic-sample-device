TEMPLATE = app

QT += qml quick
LIBS += -L../liboic -loic
LIBS += -L../libcoap -lcoap


INCLUDEPATH += ../liboic
INCLUDEPATH += ../std
INCLUDEPATH += ../libcoap

CONFIG += c++11

SOURCES += main.cpp \
    application.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    application.h
