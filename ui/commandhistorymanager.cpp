/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/commandhistorymanager.h"

CommandHistoryManager::CommandHistoryManager(QObject *parent) :
    QObject(parent),
    currentIndex(-1)
{
}

void CommandHistoryManager::commandExecuted(const QString& command)
{
    history.push_front(command);
    currentIndex = -1;
}

void CommandHistoryManager::up()
{
    if(currentIndex == int(history.size() - 1)) {
        return;
    }

    currentIndex = currentIndex + 1;

    emit setCommand(history[currentIndex]);
}

void CommandHistoryManager::down()
{
    if(currentIndex <= 0) {
        return;
    }

    currentIndex = currentIndex - 1;

    emit setCommand(history[currentIndex]);
}

void CommandHistoryManager::reset()
{
    currentIndex = -1;
}
