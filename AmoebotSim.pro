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
    main/application.h \
    script/scriptinterface.h \
    alg/legacy/movement.h \
    sim/node.h \
    sim/particle.h \
    sim/simulator.h \
    sim/system.h \
    sim/amoebotparticle.h \
    sim/amoebotsystem.h \
    sim/localparticle.h \
    sim/tile.h \
    ui/glitem.h \
    ui/commandhistorymanager.h \
    ui/visitem.h \
    alg/ring.h \
    alg/line.h \
    alg/legacy/boundedobjcoating.h \
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
    helper/randomnumbergenerator.h \
    script/scriptengine.h \
    alg/ising.h \
    alg/compression.h \
    alg/adder.h \
    alg/rectangle.h \
    alg/sierpinski.h \
    alg/matrix.h \
    alg/linesort.h \
    alg/matrix2.h \
    alg/2sitecbridge.h \
    alg/edgedetect.h \
    alg/twositeebridge.h \
    alg/infobjcoating.h \
    alg/faultrepair.h \
    alg/convexhull.h \
    alg/shapeformation.h \
    alg/holeelimination.h \
    alg/compaction.h \
    alg/demo/pulldemo.h \
    alg/demo/tokendemo.h \
    alg/swarmseparation.h \
    alg/aggregation.h

SOURCES += \
    alg/legacy/algorithm.cpp \
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
    sim/amoebotparticle.cpp \
    sim/amoebotsystem.cpp \
    sim/tile.cpp \
    sim/localparticle.cpp \
    alg/legacy/boundedobjcoating.cpp \
    alg/legacy/universalcoating.cpp \
    alg/legacy/leaderelectiondemo.cpp \
    alg/legacy/leaderelection.cpp \
    alg/legacy/legacysystem.cpp \
    alg/legacy/legacyparticle.cpp \
    helper/randomnumbergenerator.cpp \
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
    alg/2sitecbridge.cpp \
    alg/edgedetect.cpp \
    alg/twositeebridge.cpp \
    alg/infobjcoating.cpp \
    alg/faultrepair.cpp \
    alg/convexhull.cpp \
    alg/shapeformation.cpp \
    alg/holeelimination.cpp \
    alg/compaction.cpp \
    alg/demo/pulldemo.cpp \
    alg/demo/tokendemo.cpp \
    alg/swarmseparation.cpp \
    alg/aggregation.cpp

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_Inspector.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
