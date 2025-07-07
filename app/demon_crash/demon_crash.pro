QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += \
        demon_crash.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    demon_crash.h
