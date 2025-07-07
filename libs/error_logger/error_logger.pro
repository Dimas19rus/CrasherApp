QT -= gui

TEMPLATE = lib
DEFINES += LIBERRORLOGER_LIBRARY

TARGET = error

CONFIG += c++11 dll

DESTDIR = $$OUT_PWD/../app

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
