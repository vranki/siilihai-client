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
#include <siilihaiprotocol.h>
#include <forumdatabase.h>
#include <parserdatabase.h>

#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "parserengine.h"
#include "groupsubscriptiondialog.h"

class Siilihai: public QObject {
Q_OBJECT

public:
	Siilihai();
	virtual ~Siilihai();
	QSettings settings;
public slots:
	void loginWizardFinished();
	void launchSiilihai();
	void forumAdded(ForumParser fp, ForumSubscription fs);
	void loginFinished(bool success);
	void subscribeForum();
	void showSubscribeGroup(int forum);
	void showUnsubscribeForum(int forum);
	void subscribeGroupDialogFinished();
	void forumUpdated(int forum);
	void updateClicked();
	void updateClicked(int);
	void cancelClicked();
	void statusChanged(int forum, bool reloading);
	void errorDialog(QString message);
private:
	void launchMainWindow();
    void setupParserEngine(ForumSubscription &subscription);

	LoginWizard *loginWizard;
	SubscribeWizard *subscribeWizard;
	MainWindow *mainWin;
	SiilihaiProtocol protocol;
	QHash <int, ParserEngine*> engines;
	QSqlDatabase db;
	ForumDatabase fdb;
	ParserDatabase pdb;
	QString baseUrl;
};

#endif /* SIILIHAI_H_ */
