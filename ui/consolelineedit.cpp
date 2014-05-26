#include <deque>

#include <QKeyEvent>

#include "consolelineedit.h"

ConsoleLineEdit::ConsoleLineEdit(QWidget* parent) :
    QLineEdit(parent),
    index(-1)
{
}

void ConsoleLineEdit::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Up) {
        if(index < ((int) history.size()) - 1) {
            index++;
            setText(history[index]);
        }
        event->accept();
    } else if(event->key() == Qt::Key_Down) {
        if(index >= 0) {
            index--;
            if(index >= 0) {
                setText(history[index]);
            } else {
                setText("");
            }
        }
        event->accept();
    } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if(text() == "") {
            event->accept();
        } else {
            history.push_front(text());
            index = -1;
            QLineEdit::keyPressEvent(event);
        }
    } else {
        QLineEdit::keyPressEvent(event);
    }
}
