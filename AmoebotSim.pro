QT      += core gui opengl script widgets
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

QMAKE_INFO_PLIST = res/Info.plist

SOURCES +=\
        main/main.cpp\
        ui/mainwindow.cpp

HEADERS +=\
        ui/mainwindow.h

FORMS   +=\
        ui/mainwindow.ui
