QT += widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/mainwindow.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += "C:/sfml-2.6.2/include"

LIBS += -L"C:/sfml-2.6.2/lib" \
        -lsfml-graphics \
        -lsfml-window \
        -lsfml-system
QMAKE_POST_LINK += xcopy /Y /E /I \"$$system_path($$PWD/textures)\" \"$$system_path($$OUT_PWD/debug/textures)\" > NUL 2>&1 & xcopy /Y /E /I \"$$system_path($$PWD/textures)\" \"$$system_path($$OUT_PWD/release/textures)\" > NUL 2>&1
