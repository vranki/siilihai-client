#include <QApplication>
#include "siilihai.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(siilihairesources);
    QApplication siilihaiApplication(argc, argv);
    QCoreApplication::setOrganizationName("Siilihai");
    QCoreApplication::setOrganizationDomain("siilihai.com");
    QCoreApplication::setApplicationName("Siilihai");
    Siilihai sh;
    QTimer::singleShot(0, &sh, SLOT(launchSiilihai()));
    return siilihaiApplication.exec();
}
