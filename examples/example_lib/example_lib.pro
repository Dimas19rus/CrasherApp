QT -= gui

TEMPLATE = lib
DEFINES += LIBTEST_LIBRARY

TARGET = test

CONFIG += c++11 dll

# Папки для debug и release отдельно
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../../build/debug/example/
    LIBS += -L$$PWD/../../build/debug/libs/ -lerror_logger
}
CONFIG(release, debug|release) {
    DESTDIR = $$PWD/../../build/release/example/
    LIBS += -L$$PWD/../../build/release/libs/ -lerror_logger
}

SOURCES += \
    libtest.cpp

HEADERS += \
    libtest_global.h \
    libtest.h

INCLUDEPATH += $$PWD/../../libs/error_logger

LIBS += -L$$DESTDIR
QMAKE_RPATHDIR += $$DESTDIR

release {
    QMAKE_CFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.
    QMAKE_CXXFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.

    QMAKE_LFLAGS_RELEASE += -Wl,--build-id

    TARGET_PATH = $$DESTDIR/lib$${TARGET}.so.1.0.0
    message($$TARGET_PATH)
    QMAKE_POST_LINK = objcopy --only-keep-debug "$${TARGET_PATH}" "$${TARGET_PATH}.debug"; \
                      objcopy --strip-debug "$${TARGET_PATH}"; \
                      objcopy --add-gnu-debuglink="$${TARGET_PATH}.debug" "$${TARGET_PATH}"
}

unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
