QT      += core gui opengl script widgets
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

QMAKE_INFO_PLIST = res/Info.plist

SOURCES +=\
        main/main.cpp\
        main/mainwindow.cpp

HEADERS +=\
        main/mainwindow.h

FORMS   +=\
        main/mainwindow.ui
