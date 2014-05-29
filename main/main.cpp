#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ui/visitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
