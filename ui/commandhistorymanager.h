/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_UI_COMMANDHISTORYMANAGER_H_
#define AMOEBOTSIM_UI_COMMANDHISTORYMANAGER_H_

#include <deque>

#include <QObject>

class CommandHistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit CommandHistoryManager(QObject *parent = 0);

signals:
    void setCommand(const QString command);

public slots:
    void commandExecuted(const QString& command);
    void up();
    void down();
    void reset();

protected:
    std::deque<QString> history;
    int currentIndex;
};

#endif  // AMOEBOTSIM_UI_COMMANDHISTORYMANAGER_H_
