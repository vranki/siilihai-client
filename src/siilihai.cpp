/*
 * siilihai.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: vranki
 */

#include "siilihai.h"

Siilihai::Siilihai() :
	QObject(), fdb(this), pdb(this) {
	loginWizard = 0;
	mainWin = 0;
}

void Siilihai::launchSiilihai() {
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName("siilihai.db");
	if (!db.open()) {
		QMessageBox msgBox;
		msgBox.setText("Error: Unable to open database.");
		msgBox.exec();
		QCoreApplication::quit();
		return;
	}
	//protocol.setBaseURL(settings.value("network/baseurl", "http://www.siilihai.com/").toString());
	protocol.setBaseURL(settings.value("network/baseurl",
			"http://localhost:8000/").toString());
	pdb.openDatabase();
	fdb.openDatabase();
	if (settings.value("account/username", "").toString() == "") {
		loginWizard = new LoginWizard(0, protocol);
		connect(loginWizard, SIGNAL(finished(int)), this,
				SLOT(loginWizardFinished()));
	} else {
		launchMainWindow();
		mainWin->updateForumList();

		QList<ForumSubscription> forums = fdb.listSubscriptions();
		for (int i = 0; i < forums.size(); i++) {
			setupParserEngine(forums[i]);
			// @todo do something with these
			engines[forums[i].parser]->updateGroupList();
		}

		connect(&protocol, SIGNAL(loginFinished(bool)), this,
				SLOT(loginFinished(bool)));
		protocol.login(settings.value("account/username", "").toString(),
				settings.value("account/password", "").toString());
	}
}

void Siilihai::setupParserEngine(ForumSubscription &subscription) {
	ParserEngine *pe = new ParserEngine(&fdb, this);
	ForumParser parser = pdb.getParser(subscription.parser);
	pe->setParser(parser);
	pe->setSubscription(subscription);
	engines[subscription.parser] = pe;
	connect(pe, SIGNAL(groupListChanged(int)), this, SLOT(showSubscribeGroup(int)));
}

void Siilihai::loginFinished(bool success) {
	if (success) {
		if (fdb.listSubscriptions().size() == 0) {
			subscribeForum();
		}
	} else {
		QMessageBox msgBox;
		msgBox.setText(
				"Error: Login failed. Check your username, password and network connection.");
		msgBox.exec();
	}
	disconnect(&protocol, SIGNAL(loginFinished(bool)));
}

void Siilihai::subscribeForum() {
	subscribeWizard = new SubscribeWizard(mainWin, protocol);
	subscribeWizard->setModal(true);
	connect(subscribeWizard,
			SIGNAL(forumAdded(ForumParser, ForumSubscription)), this,
			SLOT(forumAdded(ForumParser, ForumSubscription)));
}

Siilihai::~Siilihai() {
	if (mainWin)
		mainWin->deleteLater();
	mainWin = 0;
}
void Siilihai::loginWizardFinished() {
	loginWizard->deleteLater();
	loginWizard = 0;
	if (settings.value("account/username", "").toString() == "") {
		QApplication::exit(-1);
	} else {
		launchMainWindow();
	}
}

void Siilihai::launchMainWindow() {
	mainWin = new MainWindow(pdb, fdb);
	connect(mainWin, SIGNAL(subscribeForum()), this, SLOT(subscribeForum()));
	mainWin->show();
}

void Siilihai::forumAdded(ForumParser fp, ForumSubscription fs) {
	if (!pdb.storeParser(fp) || !fdb.addForum(fs)) {
		QMessageBox msgBox(mainWin);
		msgBox.setText(
				"Error: Unable to subscribe to forum. Are you already subscribed?");
		msgBox.exec();
	} else {
		setupParserEngine(fs);
		engines[fs.parser]->updateGroupList();

		mainWin->updateForumList();
	}
}

void Siilihai::showSubscribeGroup(int forum) {
	QMessageBox msgBox(mainWin);
	msgBox.setText(
			"Groups have changed!");
	msgBox.exec();
}
