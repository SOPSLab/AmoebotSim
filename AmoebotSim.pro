QT      += core gui qml quick
CONFIG  += c++11
TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

HEADERS += \
    alg/legacy/algorithm.h \
    alg/legacy/algorithmwithflags.h \
    alg/legacy/infobjcoating.h \
    alg/legacy/triangle.h \
    main/application.h \
    script/scriptinterface.h \
    alg/legacy/movement.h \
    sim/node.h \
    sim/particle.h \
    sim/simulator.h \
    sim/system.h \
    ui/glitem.h \
    ui/commandhistorymanager.h \
    ui/visitem.h \
    alg/legacy/ring.h \
    alg/legacy/line.h \
    alg/legacy/square.h \
    alg/legacy/boundedobjcoating.h \
    alg/legacy/compaction.h \
    alg/legacy/holeelimstandard.h \
    alg/legacy/holeelimcompaction.h \
    alg/legacy/universalcoating.h \
    alg/legacy/leaderelectiondemo.h \
    alg/legacy/leaderelection.h \
    helper/universalcoatinghelper.h \
    helper/rect.h \
    helper/misc.h \
    helper/bipartitematchinggraph.h \
    ui/view.h \
    alg/legacy/legacysystem.h \
    alg/legacy/legacyparticle.h \
    alg/labellednocompassparticle.h \
    alg/amoebotparticle.h \
    alg/amoebotsystem.h \
    helper/randomnumbergenerator.h \
    alg/hexagon.h \
    alg/tokendemo.h \
    script/scriptengine.h \
    alg/ising.h \
    alg/compression.h \
    alg/adder.h \
    alg/rectangle.h \
    alg/sierpinski.h \
    alg/matrix.h \
    alg/linesort.h \
    alg/matrix2.h \
    alg/2sitecbridge.h

SOURCES += \
    alg/legacy/algorithm.cpp \
    alg/legacy/infobjcoating.cpp \
    alg/legacy/triangle.cpp \
    main/application.cpp \
    main/main.cpp\
    sim/particle.cpp \
    sim/simulator.cpp \
    sim/system.cpp \
    ui/commandhistorymanager.cpp \
    ui/glitem.cpp \
    ui/visitem.cpp \
    alg/legacy/ring.cpp \
    alg/legacy/line.cpp \
    alg/legacy/square.cpp \
    alg/legacy/boundedobjcoating.cpp \
    alg/legacy/compaction.cpp \
    alg/legacy/holeelimstandard.cpp \
    alg/legacy/holeelimcompaction.cpp \
    alg/legacy/universalcoating.cpp \
    alg/legacy/leaderelectiondemo.cpp \
    alg/legacy/leaderelection.cpp \
    alg/legacy/legacysystem.cpp \
    alg/legacy/legacyparticle.cpp \
    alg/labellednocompassparticle.cpp \
    alg/amoebotparticle.cpp \
    alg/amoebotsystem.cpp \
    helper/randomnumbergenerator.cpp \
    alg/hexagon.cpp \
    alg/tokendemo.cpp \
    script/scriptinterface.cpp \
    script/scriptengine.cpp \
    ui/view.cpp \
    alg/ising.cpp \
    alg/compression.cpp \
    alg/adder.cpp \
    alg/rectangle.cpp \
    alg/sierpinski.cpp \
    alg/matrix.cpp \
    alg/linesort.cpp \
    alg/matrix2.cpp \
    alg/2sitecbridge.cpp

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_Inspector.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
