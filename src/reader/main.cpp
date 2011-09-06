#include <QCoreApplication>
#include "siilihai.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(siilihairesources);
    QCoreApplication::setOrganizationName("Siilihai");
    QCoreApplication::setOrganizationDomain("siilihai.com");
    QCoreApplication::setApplicationName("Siilihai");
    Siilihai sh(argc, argv);
    QTimer::singleShot(0, &sh, SLOT(launchSiilihai()));
    return sh.exec();
}
