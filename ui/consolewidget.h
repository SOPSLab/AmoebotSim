#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QColor>
#include <QWidget>

//#include "helper/config.h"

class QPushButton;

namespace Ui {
class ConsoleWidget;
}

class ConsoleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget *parent = 0);
    ~ConsoleWidget();

signals:
    void executeCommand(const QString command);

public slots:
    void log(const QString msg, const bool isError);

private slots:
    void on_inputLineEdit_returnPressed();

protected:
    void addMsg(QString msg, QColor color = Qt::black);

private:
    Ui::ConsoleWidget *ui;

//    const Config& config = Config::get();
};

#endif // CONSOLEWIDGET_H
