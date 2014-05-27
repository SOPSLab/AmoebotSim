#include "mainwindow.h"
#include "viswindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    VisWindow* visWindow = new VisWindow();
    // create widget as a container for the visWindow
    // the widget takes over ownership
    // in turn, the widget is a child of this window
    QWidget* widget = createWindowContainer(visWindow, this);
    ui->horizontalLayout->insertWidget(0, widget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
