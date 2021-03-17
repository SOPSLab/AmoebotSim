QT      += core gui qml quick
CONFIG  += c++11
TARGET    = AmoebotSim
TEMPLATE  = app

macx:ICON = res/icon/icon.icns
QMAKE_INFO_PLIST = res/Info.plist

win32:RC_FILE = res/AmoebotSim.rc

HEADERS += \
    alg/demo/ballroomdemo.h \
    alg/demo/discodemo.h \
    alg/demo/dynamicdemo.h \
    alg/demo/metricsdemo.h \
    alg/demo/tokendemo.h \
    alg/compression.h \
    alg/energyshape.h \
    alg/energysharing.h \
    alg/infobjcoating.h \
    alg/shapeformation.h \
    core/amoebotparticle.h \
    core/amoebotsystem.h \
    core/localparticle.h \
    core/metric.h \
    core/node.h \
    core/object.h \
    core/particle.h \
    core/simulator.h \
    core/system.h \
    helper/randomnumbergenerator.h \
    main/application.h \
    script/scriptengine.h \
    script/scriptinterface.h \
    ui/algorithm.h \
    ui/glitem.h \
    ui/parameterlistmodel.h \
    ui/view.h \
    ui/visitem.h \
    alg/leaderelection.h

SOURCES += \
    alg/demo/ballroomdemo.cpp \
    alg/demo/discodemo.cpp \
    alg/demo/dynamicdemo.cpp \
    alg/demo/metricsdemo.cpp \
    alg/demo/tokendemo.cpp \
    alg/compression.cpp \
    alg/energyshape.cpp \
    alg/energysharing.cpp \
    alg/infobjcoating.cpp \
    alg/shapeformation.cpp \
    core/amoebotparticle.cpp \
    core/amoebotsystem.cpp \
    core/localparticle.cpp \
    core/metric.cpp \
    core/object.cpp \
    core/particle.cpp \
    core/simulator.cpp \
    core/system.cpp \
    helper/randomnumbergenerator.cpp \
    main/application.cpp \
    main/main.cpp\
    script/scriptengine.cpp \
    script/scriptinterface.cpp \
    ui/algorithm.cpp \
    ui/glitem.cpp \
    ui/parameterlistmodel.cpp \
    ui/view.cpp \
    ui/visitem.cpp \
    alg/leaderelection.cpp

RESOURCES += \
    res/qml.qrc \
    res/textures.qrc

OTHER_FILES += \
    res/qml/A_Button.qml \
    res/qml/A_Inspector.qml \
    res/qml/A_ResultTextField.qml \
    res/qml/main.qml
