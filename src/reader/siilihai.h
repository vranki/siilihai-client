/*
 * siilihai.h
 *
 *  Created on: Sep 25, 2009
 *      Author: vranki
 */

#ifndef SIILIHAI_H_
#define SIILIHAI_H_
#include <QObject>
#include <QSettings>
#include <QtSql>
#include <QDir>
#include <QMessageBox>
#include <QNetworkProxy>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/parserdatabase.h>
#include <siilihai/parserreport.h>
#include <siilihai/parserengine.h>

#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "groupsubscriptiondialog.h"
#include "forumlistwidget.h"
#include "reportparser.h"

#ifndef Q_WS_HILDON
#include "../parsermaker/parsermaker.h"
#else
class ParserMaker;
#endif

#define DATABASE_FILE "/.siilihai.db"
#define BASEURL "http://www.siilihai.com/"


class Siilihai: public QObject {
Q_OBJECT

public:
	Siilihai();
	virtual ~Siilihai();
public slots:
	void loginWizardFinished();
	void launchSiilihai();
	void forumAdded(ForumParser fp, ForumSubscription fs);
	void loginFinished(bool success, QString motd=QString());
	void subscribeForum();
	void showSubscribeGroup(int forum);
	void showUnsubscribeForum(int forum);
	void subscribeGroupDialogFinished();
	void forumUpdated(int forumid);
	void updateClicked();
	void updateClicked(int forumid, bool force=false);
	void cancelClicked();
	void reportClicked(int forumid);
	void statusChanged(int forumid, bool reloading, float progress);
	void errorDialog(QString message);
	void listSubscriptionsFinished(QList<int> subscriptions);
	void updateForumParser(ForumParser parser);
	void launchParserMaker();
	void parserMakerClosed();
	void sendParserReportFinished(bool success);
private:
	void launchMainWindow();
    void setupParserEngine(ForumSubscription &subscription);
    void updateState();

	LoginWizard *loginWizard;
	SubscribeWizard *subscribeWizard;
	MainWindow *mainWin;
	SiilihaiProtocol protocol;
	QHash <int, ParserEngine*> engines;
	QSqlDatabase db;
	ForumDatabase fdb;
	ParserDatabase pdb;
	QString baseUrl;
	bool loginSuccessful, offlineMode;
	QSettings settings;
	QList<int> parsersToUpdateLeft;
	ParserMaker *parserMaker;
};

#endif /* SIILIHAI_H_ */
