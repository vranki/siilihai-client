#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "siilihaiclient.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/forumdata/forumthread.h>

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Siilihai");
    QCoreApplication::setOrganizationDomain("siilihai.com");
    QCoreApplication::setApplicationName("Siilihai");
    QQmlApplicationEngine engine;
    qmlRegisterType<SiilihaiClient>("com.siilihai.siilihai", 1, 0, "SiilihaiClient");
    qRegisterMetaType<QObjectList>("QObjectList");
    qRegisterMetaType<ForumMessage*>("ForumMessage*");
    qRegisterMetaType<ForumThread*>("ForumThread*");
    qmlRegisterType<UpdateError>("org.vranki.siilihai", 1, 0, "UpdateError");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
