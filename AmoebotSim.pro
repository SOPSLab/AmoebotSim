QT      += core gui qml quick script
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/pss.rc

SOURCES += \
    main/main.cpp\
    ui/glitem.cpp \
    ui/visitem.cpp

HEADERS += \
    ui/glitem.h \
    ui/visitem.h

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/main.qml
