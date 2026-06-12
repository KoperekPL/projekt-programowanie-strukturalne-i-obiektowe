QT += widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/Enemy.cpp \
    src/Map.cpp \
    src/Game.cpp \
    src/Config.cpp \
    src/AssetManager.cpp \
    src/Tower.cpp \
    src/WaveManager.cpp \
    src/SaveManager.cpp

HEADERS += \
    src/mainwindow.h \
    src/Enemy.h \
    src/Map.h \
    src/Game.h \
    src/Tower.h \
    src/Config.h \
    src/AssetManager.h \
    src/SaveManager.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/sfml/include

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/sfml/lib \
            -lsfml-graphics-d \
            -lsfml-window-d \
            -lsfml-system-d \
            -lsfml-audio-d
} else {
    LIBS += -L$$PWD/sfml/lib \
            -lsfml-graphics \
            -lsfml-window \
            -lsfml-system \
            -lsfml-audio
}

QMAKE_POST_LINK += xcopy /Y /E /I \"$$system_path($$PWD/textures)\" \"$$system_path($$OUT_PWD/debug/textures)\" > NUL 2>&1 & xcopy /Y /E /I \"$$system_path($$PWD/textures)\" \"$$system_path($$OUT_PWD/release/textures)\" > NUL 2>&1
QMAKE_POST_LINK += & xcopy /Y /E /I \"$$system_path($$PWD/map)\" \"$$system_path($$OUT_PWD/debug/map)\" > NUL 2>&1 & xcopy /Y /E /I \"$$system_path($$PWD/map)\" \"$$system_path($$OUT_PWD/release/map)\" > NUL 2>&1
QMAKE_POST_LINK += & xcopy /Y \"$$system_path($$PWD/sfml/bin/*.dll)\" \"$$system_path($$OUT_PWD/debug/)\" > NUL 2>&1 & xcopy /Y \"$$system_path($$PWD/sfml/bin/*.dll)\" \"$$system_path($$OUT_PWD/release/)\" > NUL 2>&1
