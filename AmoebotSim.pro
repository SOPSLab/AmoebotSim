QT      += core gui qml quick script
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

SOURCES += \
    alg/algorithm.cpp \
    alg/dummyalg.cpp \
    main/application.cpp \
    main/main.cpp\
    sim/node.cpp \
    sim/particle.cpp \
    sim/simulator.cpp \
    sim/system.cpp \
    ui/glitem.cpp \
    ui/visitem.cpp

HEADERS += \
    alg/algorithm.h \
    alg/dummyalg.h \
    main/application.h \
    script/scriptinterface.h \
    sim/movement.h \
    sim/node.h \
    sim/particle.h \
    sim/simulator.h \
    sim/system.h \
    ui/glitem.h \
    ui/visitem.h

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
