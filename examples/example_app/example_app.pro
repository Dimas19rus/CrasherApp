QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG   += c++11

TARGET = MyApp

# Каталог вывода — разделяем debug и release
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../../build/debug/example/
    LIBS += -L$$PWD/../../build/debug/libs/ -lerror_logger
    LIBS += -L$$PWD/../../build/debug/example/ -ltest
    QMAKE_RPATHDIR += $$PWD/build/debug
}
CONFIG(release, debug|release) {
    DESTDIR = $$PWD/../../build/release/example/
    LIBS += -L$$PWD/../../build/release/libs/ -lerror_logger
    LIBS += -L$$PWD/../../build/release/example/ -ltest
    QMAKE_RPATHDIR += $$PWD/build/release

    QMAKE_CFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.
    QMAKE_CXXFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.

    QMAKE_LFLAGS_RELEASE += -Wl,--build-id

    TARGET_PATH = $${DESTDIR}/$${TARGET}
    # Отдельные действия после сборки релиза (примеры objcopy)
    QMAKE_POST_LINK = objcopy --only-keep-debug "$${TARGET_PATH}" "$${TARGET_PATH}.debug"; \
                      objcopy --strip-debug "$${TARGET_PATH}"; \
                      objcopy --add-gnu-debuglink="$${TARGET_PATH}.debug" "$${TARGET_PATH}"
}

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += $$PWD/../example_lib
INCLUDEPATH += $$PWD/../../libs/error_logger



qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
