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
	loginSuccessful = false;
}

void Siilihai::launchSiilihai() {
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QDir::homePath() + "/.siilihai.db");
	if (!db.open()) {
		QMessageBox msgBox;
		msgBox.setText("Error: Unable to open database.");
		msgBox.exec();
		QCoreApplication::quit();
		return;
	}
	if (true) {
		baseUrl
				= settings.value("network/baseurl", "http://www.siilihai.com/").toString();
	} else {
		baseUrl
				= settings.value("network/baseurl", "http://localhost:8000/").toString();
	}
	protocol.setBaseURL(baseUrl);
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
			// engines[forums[i].parser]->updateGroupList();
		}

		connect(&protocol, SIGNAL(loginFinished(bool, QString)), this,
				SLOT(loginFinished(bool, QString)));
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
	connect(pe, SIGNAL(groupListChanged(int)), this,
			SLOT(showSubscribeGroup(int)));
	connect(pe, SIGNAL(forumUpdated(int)), this, SLOT(forumUpdated(int)));
	connect(pe, SIGNAL(statusChanged(int, bool)), this,
			SLOT(statusChanged(int, bool)));
	connect(pe, SIGNAL(statusChanged(int, bool)), mainWin,
			SLOT(setForumStatus(int, bool)));
	connect(pe, SIGNAL(updateFailure(QString)), this,
			SLOT(errorDialog(QString)));
}

void Siilihai::loginFinished(bool success, QString motd) {
	if (success) {
		if (fdb.listSubscriptions().size() == 0) {
			subscribeForum();
		}
		loginSuccessful = true;
	} else {
		QMessageBox msgBox;
		if (motd.length() > 0) {
			msgBox.setText(motd);
		} else {
			msgBox.setText(
					"Error: Login failed. Check your username, password and network connection.");
		}
		msgBox.exec();
	}
	disconnect(&protocol, SIGNAL(loginFinished(bool, QString)));
}

void Siilihai::subscribeForum() {
	subscribeWizard = new SubscribeWizard(mainWin, protocol, baseUrl);
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
	connect(mainWin, SIGNAL(unsubscribeForum(int)), this,
			SLOT(showUnsubscribeForum(int)));
	connect(mainWin, SIGNAL(updateClicked()), this, SLOT(updateClicked()));
	connect(mainWin, SIGNAL(updateClicked(int)), this, SLOT(updateClicked(int)));
	connect(mainWin, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
	connect(mainWin, SIGNAL(groupSubscriptions(int)), this,
			SLOT(showSubscribeGroup(int)));
	connect(mainWin, SIGNAL(messageRead(ForumMessage)), &fdb,
			SLOT(markMessageRead(ForumMessage)));

	mainWin->show();
	if (loginSuccessful) {
		mainWin->updateForumList();
		if (fdb.listSubscriptions().size() == 0)
			subscribeForum();
	}
}

void Siilihai::forumAdded(ForumParser fp, ForumSubscription fs) {
	if (!pdb.storeParser(fp) || !fdb.addForum(fs)) {
		QMessageBox msgBox(mainWin);
		msgBox.setText(
				"Error: Unable to subscribe to forum. Are you already subscribed?");
		msgBox.exec();
	} else {
		setupParserEngine(fs);
		mainWin->updateForumList();
		engines[fs.parser]->updateGroupList();
	}
}

void Siilihai::errorDialog(QString message) {
	QMessageBox msgBox(mainWin);
	msgBox.setModal(true);
	msgBox.setText(message);
	msgBox.exec();
}

void Siilihai::showSubscribeGroup(int forum) {
	if (forum > 0 && loginSuccessful) {
		GroupSubscriptionDialog *gsd = new GroupSubscriptionDialog(mainWin);
		gsd->setModal(false);
		gsd->setForum(&fdb, forum);
		connect(gsd, SIGNAL(finished(int)), this,
				SLOT(subscribeGroupDialogFinished()));
		gsd->exec();
	}
}

void Siilihai::subscribeGroupDialogFinished() {
	if (loginSuccessful) {
		qDebug() << "SFD finished, updating list";
		mainWin->updateForumList();
		updateClicked();
	}
}

void Siilihai::forumUpdated(int forum) {
	if (loginSuccessful) {
		qDebug() << "Forum " << forum << " has been updated";
		mainWin->updateForumList();
	}
}
void Siilihai::updateClicked() {
	qDebug() << "Update clicked, updating all forums";
	QHashIterator<int, ParserEngine*> i(engines);
	while (i.hasNext()) {
		i.next();
		i.value()->updateForum();
	}
}

void Siilihai::updateClicked(int forumid) {
	qDebug() << "Update selected clicked, updating forum " << forumid;
	engines[forumid]->updateForum();
}

void Siilihai::cancelClicked() {
	qDebug() << "Cancel clicked, stopping all forum updates";
	QHashIterator<int, ParserEngine*> i(engines);
	while (i.hasNext()) {
		i.next();
		i.value()->cancelOperation();
	}
}

void Siilihai::statusChanged(int forum, bool reloading) {
	qDebug() << "Status change; forum" << forum << " is reloading: "
			<< reloading;
	QHashIterator<int, ParserEngine*> i(engines);
	while (i.hasNext()) {
		i.next();
		qDebug() << i.key() << " is busy: " << i.value()->isBusy();
	}
}

void Siilihai::showUnsubscribeForum(int forum) {
	if (forum > 0) {
		QMessageBox msgBox;
		msgBox.setText("Really unsubscribe from forum?");
		// msgBox.setInformativeText(fdb.getSubscription(forum).forum_name);
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		if (msgBox.exec() == QMessageBox::Yes) {
			fdb.deleteForum(forum);
			pdb.deleteParser(forum);
			mainWin->updateForumList();
			engines[forum]->deleteLater();
			engines.remove(forum);
		}
	}
}
