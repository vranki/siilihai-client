#include <QApplication>
#include "parsermaker.h"

int main(int argc, char *argv[]) {
//	Q_INIT_RESOURCE(application);
    QCoreApplication::setOrganizationName("Siilihai");
    QCoreApplication::setOrganizationDomain("siilihai.com");
    QCoreApplication::setApplicationName("Siilihai client");
	QApplication app(argc, argv);
	ParserMaker pm;
	// QTimer::singleShot(0, &sh, SLOT(launchParserMaker()));
	return app.exec();
}
