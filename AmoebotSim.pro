QT      += core gui qml quick script
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

SOURCES += \
    main/main.cpp\
    ui/glitem.cpp \
    ui/visitem.cpp \
    sim/simulator.cpp \
    sim/system.cpp \
    sim/particle.cpp \
    main/application.cpp \
    alg/algorithm.cpp \
    alg/dummyalg.cpp \
    sim/node.cpp

HEADERS += \
    ui/glitem.h \
    ui/visitem.h \
    sim/simulator.h \
    sim/system.h \
    sim/particle.h \
    main/application.h \
    alg/algorithm.h \
    sim/movement.h \
    alg/dummyalg.h \
    sim/node.h

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
