#ifndef CONSOLELINEEDIT_H
#define CONSOLELINEEDIT_H

#include <deque>

#include <QLineEdit>

class ConsoleLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit ConsoleLineEdit(QWidget* parent = 0);
    
protected:
    virtual void keyPressEvent(QKeyEvent* event);

protected:
    std::deque<QString> history;
    int index;
};

#endif // CONSOLELINEEDIT_H
