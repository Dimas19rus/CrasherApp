QT -= gui

TARGET = error_logger
TEMPLATE = lib
CONFIG += c++11 dll

DEFINES += LIBERRORLOGGER_LIBRARY


CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../../build/debug/libs/
}
CONFIG(release, debug|release) {
    DESTDIR = $$PWD/../../build/release/libs/
}


SOURCES += \
    error_handler.cpp

HEADERS += \
    crash_report_fields.h \
    error_handler.h \
    json_builder.h \
    json_reader.h \
    liberror_global.h

LIBS += -ldl #нужна

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
