QT -= gui

TEMPLATE = lib
DEFINES += LIBTEST_LIBRARY

TARGET = test

CONFIG += c++11 dll

DESTDIR = $$OUT_PWD/../app

SOURCES += \
    libtest.cpp

HEADERS += \
    libtest_global.h \
    libtest.h

INCLUDEPATH +=  ../liberror

LIBS += -L$$OUT_PWD/../app -lerror
QMAKE_RPATHDIR += $$OUT_PWD

release {
    QMAKE_CFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$${PWD}=.
    QMAKE_CXXFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$${PWD}=.

    QMAKE_LFLAGS_RELEASE += -Wl,--build-id

    TARGET_PATH = $$DESTDIR/lib$${TARGET}.so.1.0.0  # обычно имя с версией для .so
    message($$TARGET_PATH)
    QMAKE_POST_LINK = objcopy --only-keep-debug "$${TARGET_PATH}" "$${TARGET_PATH}.debug"; \
                      objcopy --strip-debug "$${TARGET_PATH}"; \
                      objcopy --add-gnu-debuglink="$${TARGET_PATH}.debug" "$${TARGET_PATH}"
}


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
