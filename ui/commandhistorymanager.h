#ifndef COMMANDHISTORYMANAGER_H
#define COMMANDHISTORYMANAGER_H

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

#endif // COMMANDHISTORYMANAGER_H
