#include "siilihai.h"

Siilihai::Siilihai() :
	QObject(), fdb(this), pdb(this) {
	loginWizard = 0;
	mainWin = 0;
	loginSuccessful = false;
	offlineMode = false;
	parserMaker = 0;
}

void Siilihai::launchSiilihai() {
	mainWin = new MainWindow(pdb, fdb, &settings);
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QDir::homePath() + DATABASE_FILE);
	if (!db.open()) {
		QMessageBox msgBox(mainWin);
		msgBox.setText("Error: Unable to open database.");
		msgBox.exec();
		QCoreApplication::quit();
		return;
	}
	QString proxy = settings.value("preferences/http_proxy", "").toString();
	if (proxy.length() > 0) {
		QUrl proxyUrl = QUrl(proxy);
		if (proxyUrl.isValid()) {
			QNetworkProxy nproxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyUrl.port(0));
			QNetworkProxy::setApplicationProxy(nproxy);
		} else {
			errorDialog("Warning: http proxy is not valid URL");
		}
	}
	baseUrl = settings.value("network/baseurl", BASEURL).toString();
	protocol.setBaseURL(baseUrl);
	pdb.openDatabase();
	fdb.openDatabase();

#ifdef Q_WS_HILDON
	if(settings.value("firstrun", true).toBool()) {
		errorDialog("This is beta software\nMaemo version can't connect\n"
				"to the Internet, so please do it manually before continuing.\n"
				"The UI is also work in progress. Go to www.siilihai.com for details.");
		settings.setValue("firstrun", false);
	}
#endif
	if (settings.value("account/username", "").toString() == "") {
		loginWizard = new LoginWizard(mainWin, protocol, settings);
		connect(loginWizard, SIGNAL(finished(int)), this,
				SLOT(loginWizardFinished()));
	} else {
		launchMainWindow();

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
	connect(pe, SIGNAL(statusChanged(int, bool, float)), this,
			SLOT(statusChanged(int, bool, float)));
	connect(pe, SIGNAL(statusChanged(int, bool, float)), mainWin,
			SLOT(setForumStatus(int, bool, float)));
	connect(pe, SIGNAL(updateFailure(QString)), this,
			SLOT(errorDialog(QString)));
}

void Siilihai::loginFinished(bool success, QString motd) {
	if (success) {
		connect(&protocol, SIGNAL(listSubscriptionsFinished(QList<int>)), this,
				SLOT(listSubscriptionsFinished(QList<int>)));
		connect(&protocol, SIGNAL(sendParserReportFinished(bool)), this,
				SLOT(sendParserReportFinished(bool)));

		protocol.listSubscriptions();

		loginSuccessful = true;
	} else {
		QMessageBox msgBox(mainWin);
		msgBox.setModal(true);
		if (motd.length() > 0) {
			msgBox.setText(motd);
		} else {
			msgBox.setText(
					"Error: Login failed. Check your username, password and network connection.\nWorking offline.");
		}
		msgBox.exec();
		offlineMode = true;
		updateState();
	}
	disconnect(&protocol, SIGNAL(loginFinished(bool, QString)));
}

void Siilihai::listSubscriptionsFinished(QList<int> subscriptions) {
	QList<ForumSubscription> dbSubscriptions = fdb.listSubscriptions();
	QList<int> unsubscribedForums;
	for (int d = 0; d < dbSubscriptions.size(); d++) {
		bool found = false;
		for (int i = 0; i < subscriptions.size(); i++) {
			if (subscriptions.at(i) == dbSubscriptions.at(d).parser)
				found = true;
		}
		if (!found) {
			qDebug() << "Site says not subscribed to "
					<< dbSubscriptions.at(d).toString();
			unsubscribedForums.append(dbSubscriptions.at(d).parser);
		}
	}
	for (int i = 0; i < unsubscribedForums.size(); i++) {
		fdb.deleteForum(unsubscribedForums.at(i));
		pdb.deleteParser(unsubscribedForums.at(i));
		mainWin->forumList()->updateForumList();
		engines[unsubscribedForums.at(i)]->deleteLater();
		engines.remove(unsubscribedForums.at(i));

		qDebug() << "Deleted forum " << unsubscribedForums.at(i);
	}

	if (dbSubscriptions.size() == 0) {
		subscribeForum();
	} else { // Update parser def's
		dbSubscriptions = fdb.listSubscriptions();
		parsersToUpdateLeft.clear();
		connect(&protocol, SIGNAL(getParserFinished(ForumParser)), this,
				SLOT(updateForumParser(ForumParser)));
		for (int d = 0; d < dbSubscriptions.size(); d++) {
			parsersToUpdateLeft.append(dbSubscriptions.at(d).parser);
			emit statusChanged(dbSubscriptions.at(d).parser, true, -1);
		}
		if (parsersToUpdateLeft.size() > 0) {
			protocol.getParser(parsersToUpdateLeft.at(0));
		}
	}
	updateState();
}

void Siilihai::updateForumParser(ForumParser parser) {
	if (parser.isSane()) {
		pdb.storeParser(parser);
		emit
		statusChanged(parser.id, false, -1);
		parsersToUpdateLeft.removeFirst();
		if (parsersToUpdateLeft.isEmpty()) {
			disconnect(&protocol, SIGNAL(getParserFinished(ForumParser)), this,
					SLOT(updateForumParser(ForumParser)));
			if (settings.value("preferences/update_automatically", false).toBool())
				updateClicked();
		} else {
			protocol.getParser(parsersToUpdateLeft.first());
		}
		updateState();
	}
}

void Siilihai::updateState() {
	bool ready = parsersToUpdateLeft.isEmpty() && loginSuccessful;
	mainWin->setReaderReady(ready, offlineMode);
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
	if (settings.value("account/username", "").toString().length()==0) {
		qDebug() << "Settings wizard failed, quitting.";
		QApplication::exit(-1);
	} else {
		launchMainWindow();
		loginFinished(true);
	}
}

void Siilihai::launchMainWindow() {
	connect(mainWin, SIGNAL(subscribeForum()), this, SLOT(subscribeForum()));
	connect(mainWin, SIGNAL(unsubscribeForum(int)), this,
			SLOT(showUnsubscribeForum(int)));
	connect(mainWin, SIGNAL(updateClicked()), this, SLOT(updateClicked()));
	connect(mainWin, SIGNAL(updateClicked(int,bool)), this,
			SLOT(updateClicked(int,bool)));
	connect(mainWin, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
	connect(mainWin, SIGNAL(groupSubscriptions(int)), this,
			SLOT(showSubscribeGroup(int)));
	connect(mainWin, SIGNAL(reportClicked(int)), this, SLOT(reportClicked(int)));
	connect(mainWin->threadList(), SIGNAL(messageSelected(const ForumMessage&)), &fdb,
			SLOT(markMessageRead(const ForumMessage&)));
	connect(mainWin, SIGNAL(launchParserMaker()), this,
			SLOT(launchParserMaker()));

	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		setupParserEngine(forums[i]);
	}
	mainWin->forumList()->updateForumList();
	if (loginSuccessful) {
		if (fdb.listSubscriptions().size() == 0)
			subscribeForum();
	}
	mainWin->setReaderReady(false, offlineMode);
	mainWin->show();
}

void Siilihai::forumAdded(ForumParser fp, ForumSubscription fs) {
	if (!pdb.storeParser(fp) || !fdb.addForum(fs)) {
		QMessageBox msgBox(mainWin);
		msgBox.setText(
				"Error: Unable to subscribe to forum. Are you already subscribed?");
		msgBox.exec();
	} else {
		protocol.subscribeForum(fs);
		setupParserEngine(fs);
		mainWin->forumList()->updateForumList();
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
		mainWin->forumList()->updateForumList();
		updateClicked();
	}
}

void Siilihai::forumUpdated(int forum) {
	if (loginSuccessful) {
		qDebug() << "Forum " << forum << " has been updated";
		mainWin->forumList()->updateForumList();
	}
}

void Siilihai::updateClicked() {
	qDebug() << "Update clicked, updating all forums";
	mainWin->threadList()->groupSelected(ForumGroup());

	QHashIterator<int, ParserEngine*> i(engines);
	while (i.hasNext()) {
		i.next();
		i.value()->updateForum();
	}
}

void Siilihai::updateClicked(int forumid, bool force) {
	qDebug() << "Update selected clicked, updating forum " << forumid
			<< ", force=" << force;
	engines[forumid]->updateForum(force);
}

void Siilihai::cancelClicked() {
	qDebug() << "Cancel clicked, stopping all forum updates";
	QHashIterator<int, ParserEngine*> i(engines);
	while (i.hasNext()) {
		i.next();
		i.value()->cancelOperation();
	}
}

void Siilihai::reportClicked(int forumid) {
	if (forumid > 0) {
		ForumParser parserToReport = pdb.getParser(forumid);
		ReportParser *rpt = new ReportParser(mainWin, forumid,
				parserToReport.parser_name);
		connect(rpt, SIGNAL(parserReport(ParserReport)), &protocol,
				SLOT(sendParserReport(ParserReport)));
		rpt->exec();
	}
}

void Siilihai::statusChanged(int forumid, bool reloading, float progress) {
	updateState();
}

void Siilihai::showUnsubscribeForum(int forum) {
	if (forum > 0) {
		ForumSubscription fs = fdb.getSubscription(forum);
		QMessageBox msgBox(mainWin);
		msgBox.setText("Really unsubscribe from forum?");
		msgBox.setInformativeText(fs.name);
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		if (msgBox.exec() == QMessageBox::Yes) {
			fdb.deleteForum(forum);
			pdb.deleteParser(forum);
			mainWin->forumList()->updateForumList();
			engines[forum]->deleteLater();
			engines.remove(forum);
			protocol.subscribeForum(fs, true);
		}
	}
}

void Siilihai::launchParserMaker() {

#ifndef Q_WS_HILDON
	if (!parserMaker) {
		parserMaker = new ParserMaker(mainWin, pdb, settings, protocol);
		connect(parserMaker, SIGNAL(destroyed()), this,
				SLOT(parserMakerClosed()));
	} else {
		parserMaker->showNormal();
	}
#endif
}
void Siilihai::parserMakerClosed() {
	parserMaker = 0;
}

void Siilihai::sendParserReportFinished(bool success) {
	if (!success) {
		errorDialog("Sending report failed - please try again.");
	} else {
		errorDialog("Thanks for your report");
	}
}
