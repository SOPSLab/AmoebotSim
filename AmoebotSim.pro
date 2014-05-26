QT      += core gui opengl script widgets
CONFIG  += c++11

TARGET    = AmoebotSim
TEMPLATE  = app

QMAKE_INFO_PLIST = Info.plist

SOURCES +=\
        main.cpp\
        mainwindow.cpp

HEADERS +=\
        mainwindow.h

FORMS   +=\
        mainwindow.ui
