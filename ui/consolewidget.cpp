#include <QFont>
#include <QFontMetrics>
#include <QScrollBar>

#include "consolewidget.h"
#include "ui_consolewidget.h"

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QString fontFamily("Menlo");
    int fontSize = 12;
#else
    QString fontFamily("Consolas");
    int fontSize = 8;
#endif

    QFont font = QFont(fontFamily);
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(fontSize);

    QString tabWidthString = QString("aaaa");
    int tabWidth = QFontMetrics(font).boundingRect(tabWidthString).width();

    ui->inputLineEdit->setFont(font);

    ui->logTextBrowser->setFont(font);
    ui->logTextBrowser->setTabStopWidth(tabWidth);
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
