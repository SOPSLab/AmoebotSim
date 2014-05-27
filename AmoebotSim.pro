QT      += core gui script widgets
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

QMAKE_INFO_PLIST = res/Info.plist

SOURCES += \
    main/main.cpp\
    ui/consolelineedit.cpp \
    ui/consolewidget.cpp \
    ui/glwindow.cpp \
    ui/mainwindow.cpp \
    ui/viswindow.cpp

HEADERS += \
    ui/consolelineedit.h \
    ui/consolewidget.h \
    ui/mainwindow.h \
    ui/glwindow.h \
    ui/viswindow.h

FORMS   += \
    ui/consolewidget.ui \
    ui/mainwindow.ui


RESOURCES += \
    res/textures.qrc
