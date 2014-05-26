#include <qapplication.h>
#include <QFont>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QScrollBar>

#include "consolewidget.h"
#include "ui_consolewidget.h"

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);

//    QFont font = config.global.font;
//    ui->inputLineEdit->setFont(font);

//    ui->logTextBrowser->setFont(font);
//    QString tabWidth = QString("a").repeated(config.global.tabStopWidth);
//    ui->logTextBrowser->setTabStopWidth(QFontMetrics(config.global.font).boundingRect(tabWidth).width());

    ui->logTextBrowser->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
}

ConsoleWidget::~ConsoleWidget()
{
    delete ui;
}

void ConsoleWidget::log(const QString msg, const bool isError)
{
    if(isError) {
        addMsg(msg, Qt::red);
    } else {
        addMsg(msg, Qt::blue);
    }
}

void ConsoleWidget::on_inputLineEdit_returnPressed()
{
    addMsg(ui->inputLineEdit->text());
    emit executeCommand(ui->inputLineEdit->text());
    ui->inputLineEdit->clear();
}

void ConsoleWidget::addMsg(QString msg, QColor color)
{
    ui->logTextBrowser->setTextColor(color);
    ui->logTextBrowser->append(msg);

    auto scrollBar = ui->logTextBrowser->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
