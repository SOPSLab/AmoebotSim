QT      += core gui opengl script widgets
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

QMAKE_INFO_PLIST = res/Info.plist

SOURCES +=\
    main/main.cpp\
    ui/mainwindow.cpp \
    ui/consolelineedit.cpp \
    ui/consolewidget.cpp \
    ui/viswidget.cpp

HEADERS +=\
    ui/mainwindow.h \
    ui/consolelineedit.h \
    ui/consolewidget.h \
    ui/viswidget.h

FORMS   +=\
    ui/mainwindow.ui \
    ui/consolewidget.ui

RESOURCES += \
    res/textures.qrc
