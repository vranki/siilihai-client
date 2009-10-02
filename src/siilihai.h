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
	void forumUpdated(int forum);
	void updateClicked();
	void statusChanged(int forum, bool reloading);
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
};

#endif /* SIILIHAI_H_ */
