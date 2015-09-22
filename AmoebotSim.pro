QT      += core gui qml quick
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

HEADERS += \
    alg/algorithm.h \
    alg/algorithmwithflags.h \
    alg/examplealgorithm.h \
    alg/hexagon.h \
    alg/infobjcoating.h \
    alg/triangle.h \
    main/application.h \
    script/scriptinterface.h \
    sim/movement.h \
    sim/node.h \
    sim/particle.h \
    sim/simulator.h \
    sim/system.h \
    ui/glitem.h \
    ui/commandhistorymanager.h \
    ui/visitem.h \
    alg/ring.h \
    alg/line.h \
    alg/square.h \
    alg/boundedobjcoating.h \
    alg/compaction.h \
    alg/holeelimstandard.h \
    alg/holeelimcompaction.h \
    alg/universalcoating.h \
    alg/leaderelectiondemo.h \
    alg/leaderelection.h \
    helper/universalcoatinghelper.h \
    helper/rect.h \
    helper/misc.h \
    helper/bipartitematchinggraph.h \
    ui/view.h

SOURCES += \
    alg/algorithm.cpp \
    alg/examplealgorithm.cpp \
    alg/hexagon.cpp \
    alg/infobjcoating.cpp \
    alg/triangle.cpp \
    main/application.cpp \
    main/main.cpp\
    sim/particle.cpp \
    sim/simulator.cpp \
    sim/system.cpp \
    ui/commandhistorymanager.cpp \
    ui/glitem.cpp \
    ui/visitem.cpp \
    alg/ring.cpp \
    alg/line.cpp \
    alg/square.cpp \
    alg/boundedobjcoating.cpp \
    alg/compaction.cpp \
    alg/holeelimstandard.cpp \
    alg/holeelimcompaction.cpp \
    alg/universalcoating.cpp \
    alg/leaderelectiondemo.cpp \
    alg/leaderelection.cpp

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
