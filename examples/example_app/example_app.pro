QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = MyApp

#Это для того чтобы отделить debug файл от релиза
release {
    QMAKE_CFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.
    QMAKE_CXXFLAGS_RELEASE += -g -O2 -fdebug-prefix-map=$$PWD=.

    QMAKE_LFLAGS_RELEASE += -Wl,--build-id

    TARGET_PATH = $$OUT_PWD/$$TARGET
    message($$TARGET_PATH)
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



INCLUDEPATH += ../libtest ../liberror

LIBS += -L$$OUT_PWD -ltest -lerror
QMAKE_RPATHDIR += $$OUT_PWD

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
