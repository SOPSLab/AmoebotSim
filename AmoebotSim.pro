QT      += core gui qml quick
CONFIG  += c++11
TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

HEADERS += \
    alg/demo/discodemo.h \
    alg/demo/pulldemo.h \
    alg/demo/tokendemo.h \
    alg/legacy/algorithm.h \
    alg/legacy/algorithmwithflags.h \
    alg/legacy/boundedobjcoating.h \
    alg/legacy/leaderelection.h \
    alg/legacy/leaderelectiondemo.h \
    alg/legacy/legacysystem.h \
    alg/legacy/legacyparticle.h \
    alg/legacy/movement.h \
    alg/legacy/universalcoating.h \
    alg/aggregation.h \
    alg/compaction.h \
    alg/compression.h \
    alg/convexhull.h \
    alg/edgedetect.h \
    alg/faultrepair.h \
    alg/holeelimination.h \
    alg/infobjcoating.h \
    alg/line.h \
    alg/matrix.h \
    alg/matrix2.h \
    alg/rectangle.h \
    alg/ring.h \
    alg/shapeformation.h \
    alg/sierpinski.h \
    alg/swarmseparation.h \
    core/amoebotparticle.h \
    core/amoebotsystem.h \
    core/localparticle.h \
    core/node.h \
    core/object.h \
    core/particle.h \
    core/simulator.h \
    core/system.h \
    helper/bipartitematchinggraph.h \
    helper/misc.h \
    helper/randomnumbergenerator.h \
    helper/rect.h \
    helper/universalcoatinghelper.h \
    main/application.h \
    script/scriptengine.h \
    script/scriptinterface.h \
    ui/alg.h \
    ui/commandhistorymanager.h \
    ui/glitem.h \
    ui/parameterlistmodel.h \
    ui/view.h \
    ui/visitem.h

SOURCES += \
    alg/demo/discodemo.cpp \
    alg/demo/pulldemo.cpp \
    alg/demo/tokendemo.cpp \
    alg/legacy/algorithm.cpp \
    alg/legacy/boundedobjcoating.cpp \
    alg/legacy/leaderelection.cpp \
    alg/legacy/leaderelectiondemo.cpp \
    alg/legacy/legacyparticle.cpp \
    alg/legacy/legacysystem.cpp \
    alg/legacy/universalcoating.cpp \
    alg/aggregation.cpp \
    alg/compaction.cpp \
    alg/compression.cpp \
    alg/convexhull.cpp \
    alg/edgedetect.cpp \
    alg/faultrepair.cpp \
    alg/holeelimination.cpp \
    alg/infobjcoating.cpp \
    alg/line.cpp \
    alg/matrix.cpp \
    alg/matrix2.cpp \
    alg/rectangle.cpp \
    alg/ring.cpp \
    alg/shapeformation.cpp \
    alg/sierpinski.cpp \
    alg/swarmseparation.cpp \
    core/amoebotparticle.cpp \
    core/amoebotsystem.cpp \
    core/localparticle.cpp \
    core/object.cpp \
    core/particle.cpp \
    core/simulator.cpp \
    core/system.cpp \
    helper/randomnumbergenerator.cpp \
    main/application.cpp \
    main/main.cpp\
    script/scriptengine.cpp \
    script/scriptinterface.cpp \
    ui/alg.cpp \
    ui/commandhistorymanager.cpp \
    ui/glitem.cpp \
    ui/parameterlistmodel.cpp \
    ui/view.cpp \
    ui/visitem.cpp

RESOURCES += \
    res/inputs.qrc \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_Inspector.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/A_TextField.qml \
    res/qml/main.qml
