QT -= gui
CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = demon_crash
TEMPLATE = app

SOURCES += demon_crash.cpp main.cpp
HEADERS += demon_crash.h

# INCLUDEPATH на исходники error_logger (исходники в libs/error_logger)
INCLUDEPATH += $$PWD/../../libs/error_logger

# Путь к скомпилированной библиотеке error_logger
# error_logger собирается в build/debug/libs или build/release/libs, а demon_crash в build/debug/demonCrash или build/release/demonCrash

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../build/debug/libs/ -lerror_logger
    DESTDIR = $$PWD/../../build/debug/demonCrash
}
CONFIG(release, debug|release) {
    LIBS += -L$$PWD/../../build/release/libs/ -lerror_logger
    DESTDIR = $$PWD/../../build/release/demonCrash
}

# Для запуска с правильным rpath (runtime linker)
unix {
    debug {
        QMAKE_RPATHDIR += $$PWD/../../build/debug/libs
    }
    release {
        QMAKE_RPATHDIR += $$PWD/../../build/release/libs
    }
}

DEFINES += LIBERRORLOGGER_LIBRARY
