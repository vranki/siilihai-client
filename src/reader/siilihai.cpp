#include "siilihai.h"

Siilihai::Siilihai() :
	QObject(), fdb(this), pdb(this), syncmaster(this, fdb, protocol) {
    loginWizard = 0;
    mainWin = 0;
    readerReady = false;
    offlineMode = false;
    quitting = false;
    parserMaker = 0;
    loginProgress = 0;
    groupSubscriptionDialog = 0;
    subscribeWizard = 0;
}

void Siilihai::launchSiilihai() {
    mainWin = new MainWindow(pdb, fdb, &settings);
    bool firstRun = settings.value("first_run", true).toBool();

    settings.setValue("first_run", false);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::homePath() + DATABASE_FILE);
    if (!db.open()) {
        QMessageBox msgBox(mainWin);
        msgBox.setText("Error: Unable to open database.");
        msgBox.exec();
        haltSiilihai();
        return;
    }
    QString proxy = settings.value("preferences/http_proxy", "").toString();
    if (proxy.length() > 0) {
        QUrl proxyUrl = QUrl(proxy);
        if (proxyUrl.isValid()) {
            QNetworkProxy nproxy(QNetworkProxy::HttpProxy, proxyUrl.host(),
                                 proxyUrl.port(0));
            QNetworkProxy::setApplicationProxy(nproxy);
        } else {
            errorDialog("Warning: http proxy is not valid URL");
        }
    }
    baseUrl = settings.value("network/baseurl", BASEURL).toString();
    syncEnabled = settings.value("preferences/sync_enabled", false).toBool();
    connect(&syncmaster, SIGNAL(syncFinished(bool)), this, SLOT(syncFinished(bool)));
    protocol.setBaseURL(baseUrl);
    int mySchema = settings.value("forum_database_schema", 0).toInt();
    if(!firstRun && fdb.schemaVersion() != mySchema) {
        errorDialog("The database schema has been changed. Your forum database will be reset."
                    " Sorry. ");
        fdb.resetDatabase();
    }
    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), this, SLOT(subscriptionFound(ForumSubscription*)));
    connect(&fdb, SIGNAL(subscriptionDeleted(ForumSubscription*)), this, SLOT(subscriptionDeleted(ForumSubscription*)));
    if(fdb.openDatabase()) {
        settings.setValue("forum_database_schema", fdb.schemaVersion());
    } else {
        errorDialog("Error opening Siilihai's database!\nSee console for details. Sorry.");
    }
    pdb.openDatabase();

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
        tryLogin();
    }
}

void Siilihai::tryLogin() {
    if (!loginProgress) {
        loginProgress = new QProgressDialog("Logging in..", "Cancel", 0, 100,
                                            mainWin);
        loginProgress->setWindowModality(Qt::WindowModal);
        loginProgress->setValue(0);
        connect(loginProgress, SIGNAL(canceled()), this, SLOT(haltSiilihai()));
    }
    connect(&protocol, SIGNAL(loginFinished(bool, QString)), this,
            SLOT(loginFinished(bool, QString)));
    protocol.login(settings.value("account/username", "").toString(),
                   settings.value("account/password", "").toString());
}

void Siilihai::haltSiilihai() {
    // Wtf, crashes if canceling login
    qDebug() << Q_FUNC_INFO;
    quitting = true;
    offlineModeSet(true);
    mainWin->setReaderReady(false, false);
    if(syncEnabled) {
        syncmaster.endSync();
    } else {
        // Add some stuff here later
        QCoreApplication::quit();
    }
}

void Siilihai::syncFinished(bool success){
    qDebug() << Q_FUNC_INFO << success;
    if(quitting) {
        syncEnabled = false;
        haltSiilihai();
    }
}

void Siilihai::offlineModeSet(bool newOffline) {
    qDebug() << Q_FUNC_INFO << newOffline;
    if (offlineMode && !newOffline) {
        tryLogin();
    } else if (!offlineMode && newOffline) {
        offlineMode = true;
        updateState();
    }
}

void Siilihai::loginFinished(bool success, QString motd) {
    qDebug() << Q_FUNC_INFO << success;
    disconnect(&protocol, SIGNAL(loginFinished(bool, QString)), this,
               SLOT(loginFinished(bool, QString)));
    offlineMode = !success;
    if (success) {
        connect(&protocol, SIGNAL(listSubscriptionsFinished(QList<int>)), this,
                SLOT(listSubscriptionsFinished(QList<int>)));
        connect(&protocol, SIGNAL(sendParserReportFinished(bool)), this,
                SLOT(sendParserReportFinished(bool)));

        protocol.listSubscriptions();
        if (loginProgress)
            loginProgress->setValue(30);
    } else {
        if (loginProgress) {
            loginProgress->cancel();
            loginProgress->deleteLater();
            loginProgress = 0;
        }
        QMessageBox msgBox(mainWin);
        msgBox.setModal(true);
        if (motd.length() > 0) {
            msgBox.setText(motd);
        } else {
            msgBox.setText(
                    "Error: Login failed. Check your username, password and network connection.\nWorking offline.");
        }
        msgBox.exec();
        readerReady = true;
    }
    updateState();
}

void Siilihai::listSubscriptionsFinished(QList<int> serversSubscriptions) {
    qDebug() << Q_FUNC_INFO;
    disconnect(&protocol, SIGNAL(listSubscriptionsFinished(QList<int>)), this,
               SLOT(listSubscriptionsFinished(QList<int>)));
    if (loginProgress)
        loginProgress->setValue(50);

    QList<ForumSubscription*> unsubscribedForums;
    foreach(ForumSubscription* sub, fdb.listSubscriptions()) {
        bool found = false;
        foreach(int serverSubscriptionId, serversSubscriptions) {
            if (serverSubscriptionId == sub->parser())
                found = true;
        }
        if (!found) {
            qDebug() << "Server says not subscribed to "
                    << sub->toString();
            // @TODO really should unsubscribe!
            // unsubscribedForums.append(dbSubscriptions.at(d));
        }
    }
    foreach (ForumSubscription *sub, unsubscribedForums) {
        qDebug() << "Deleting forum " << sub->toString() << "as server says it's not subscribed";
        pdb.deleteParser(sub->parser());
        fdb.deleteForum(sub);
    }

    if (fdb.listSubscriptions().isEmpty()) { // Display subscribe dialog if none subscribed
        readerReady = true;
        updateState();
        subscribeForum();
    } else { // Update parser def's
        parsersToUpdateLeft.clear();
        connect(&protocol, SIGNAL(getParserFinished(ForumParser)), this,
                SLOT(updateForumParser(ForumParser)));
        foreach(ForumSubscription *sub, fdb.listSubscriptions()) {
            parsersToUpdateLeft.append(sub);
            emit statusChanged(sub, true, -1);
        }
        if (!parsersToUpdateLeft.isEmpty()) {
            protocol.getParser(parsersToUpdateLeft.at(0)->parser());
        } else {
            readerReady = true;
            if(syncEnabled)
                syncmaster.startSync();
        }
    }
    updateState();
}

// Stores the parser if subscribed to it and updates the engine
void Siilihai::updateForumParser(ForumParser parser) {
    qDebug() << Q_FUNC_INFO;
    if (parser.isSane()) {
        foreach(ForumSubscription *sub, engines.keys()) { // Find subscription that uses parser
            if (sub->parser() == parser.id) {
                pdb.storeParser(parser);
                engines[sub]->setParser(parser);
                emit statusChanged(sub, false, -1);
                if (!parsersToUpdateLeft.isEmpty() && parsersToUpdateLeft.first()->parser()
                    == parser.id)
                    parsersToUpdateLeft.removeFirst();
                if (parsersToUpdateLeft.isEmpty()) {
                    disconnect(&protocol, SIGNAL(getParserFinished(ForumParser)),
                               this, SLOT(updateForumParser(ForumParser)));
                    readerReady = true;
                    if(syncEnabled)
                        syncmaster.startSync();

                    if (settings.value("preferences/update_automatically", false).toBool())
                        updateClicked();
                } else {
                    if (parsersToUpdateLeft.size() < 2) {
                        if (loginProgress)
                            loginProgress->setValue(80);
                    }
                    protocol.getParser(parsersToUpdateLeft.first()->parser());
                }
                updateState();
            } else {
                qDebug() << "WTF: Not subscribed to this forum, won't update.";
            }
        }
    }
}

void Siilihai::updateState() {
    qDebug() << Q_FUNC_INFO << readerReady << parsersToUpdateLeft.size();
    bool ready = parsersToUpdateLeft.isEmpty() && readerReady;
    mainWin->setReaderReady(ready, offlineMode);
    if (ready && loginProgress) {
        loginProgress->cancel();
        loginProgress->deleteLater();
        loginProgress = 0;
    }
}

void Siilihai::subscribeForum() {
    subscribeWizard = new SubscribeWizard(mainWin, protocol, baseUrl);
    subscribeWizard->setModal(true);
    connect(subscribeWizard,
            SIGNAL(forumAdded(ForumParser, ForumSubscription*)), this,
            SLOT(forumAdded(ForumParser, ForumSubscription*)));
}

Siilihai::~Siilihai() {
    if (mainWin)
        mainWin->deleteLater();
    mainWin = 0;
}

void Siilihai::loginWizardFinished() {
    loginWizard->deleteLater();
    loginWizard = 0;
    if (settings.value("account/username", "").toString().length() == 0) {
        qDebug() << "Settings wizard failed, quitting.";
        haltSiilihai();
    } else {
        launchMainWindow();
        loginFinished(true);
    }
}

void Siilihai::launchMainWindow() {
    connect(mainWin, SIGNAL(subscribeForum()), this, SLOT(subscribeForum()));
    connect(mainWin, SIGNAL(unsubscribeForum(ForumSubscription*)), this,
            SLOT(showUnsubscribeForum(ForumSubscription*)));
    connect(mainWin, SIGNAL(updateClicked()), this, SLOT(updateClicked()));
    connect(mainWin, SIGNAL(updateClicked(ForumSubscription*,bool)), this,
            SLOT(updateClicked(ForumSubscription*,bool)));
    connect(mainWin, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    connect(mainWin, SIGNAL(groupSubscriptions(ForumSubscription*)), this,
            SLOT(showSubscribeGroup(ForumSubscription*)));
    connect(mainWin, SIGNAL(reportClicked(ForumSubscription*)), this, SLOT(reportClicked(ForumSubscription*)));
    connect(mainWin->threadList(),
            SIGNAL(messageSelected(ForumMessage*)), &fdb,
            SLOT(markMessageRead(ForumMessage*)));
    connect(mainWin, SIGNAL(launchParserMaker()), this,
            SLOT(launchParserMaker()));
    connect(mainWin, SIGNAL(offlineModeSet(bool)), this,
            SLOT(offlineModeSet(bool)));
    connect(mainWin, SIGNAL(haltRequest()), this,
            SLOT(haltSiilihai()));

    if (readerReady) {
        if (fdb.listSubscriptions().size() == 0)
            subscribeForum();
    }
    mainWin->setReaderReady(false, offlineMode);
    mainWin->show();
}

void Siilihai::forumAdded(ForumParser fp, ForumSubscription *fs) {
    qDebug() << Q_FUNC_INFO << fp.toString() << fs->toString();
    if(subscribeWizard) {
        subscribeWizard->deleteLater();
        subscribeWizard = 0;
    }
    ForumSubscription *newSubscription = 0;
    if (!pdb.storeParser(fp) || !(newSubscription = fdb.addForum(fs))) {
        QMessageBox msgBox(mainWin);
        msgBox.setText(
                "Error: Unable to subscribe to forum. Are you already subscribed?");
        msgBox.exec();
    } else {
        protocol.subscribeForum(newSubscription);
        engines[newSubscription]->updateGroupList();
    }
}

void Siilihai::subscriptionFound(ForumSubscription *sub) {
    ParserEngine *pe = new ParserEngine(&fdb, this);
    ForumParser parser = pdb.getParser(sub->parser());
    pe->setParser(parser);
    pe->setSubscription(sub);
    engines[sub] = pe;
    connect(pe, SIGNAL(groupListChanged(ForumSubscription*)), this,
            SLOT(showSubscribeGroup(ForumSubscription*)));
    connect(pe, SIGNAL(forumUpdated(ForumSubscription*)), this, SLOT(forumUpdated(ForumSubscription*)));
    connect(pe, SIGNAL(statusChanged(ForumSubscription*, bool, float)), this,
            SLOT(statusChanged(ForumSubscription*, bool, float)));
    connect(pe, SIGNAL(statusChanged(ForumSubscription*, bool, float)), mainWin,
            SLOT(setForumStatus(ForumSubscription*, bool, float)));
    connect(pe, SIGNAL(updateFailure(QString)), this,
            SLOT(errorDialog(QString)));
}

void Siilihai::subscriptionDeleted(ForumSubscription *sub) {
    Q_ASSERT(engines.contains(sub));
    engines[sub]->cancelOperation();
    engines[sub]->deleteLater();
    engines.remove(sub);
}

void Siilihai::errorDialog(QString message) {
    QMessageBox msgBox(mainWin);
    msgBox.setModal(true);
    msgBox.setText(message);
    msgBox.exec();
}

void Siilihai::showSubscribeGroup(ForumSubscription* forum) {
    Q_ASSERT(forum);
    if (readerReady) {
        groupSubscriptionDialog = new GroupSubscriptionDialog(mainWin);
        groupSubscriptionDialog->setModal(false);
        groupSubscriptionDialog->setForum(&fdb, forum);
        connect(groupSubscriptionDialog, SIGNAL(finished(int)), this,
                SLOT(subscribeGroupDialogFinished()));
        groupSubscriptionDialog->exec();
    }
}

void Siilihai::subscribeGroupDialogFinished() {
    groupSubscriptionDialog->deleteLater();
    groupSubscriptionDialog = 0;
    if (readerReady) {
        qDebug() << "SFD finished, updating list";
        updateClicked();
    }
}

void Siilihai::forumUpdated(ForumSubscription* forum) {
    if (readerReady) {
        qDebug() << "Forum " << forum << " has been updated";
    }
}

void Siilihai::updateClicked() {
    foreach(ParserEngine* engine, engines.values())
        engine->updateForum();
}

void Siilihai::updateClicked(ForumSubscription* sub , bool force) {
    qDebug() << Q_FUNC_INFO << "Update selected clicked, updating forum " << sub->toString()
            << ", force=" << force;
    Q_ASSERT(engines.contains(sub));
    engines[sub]->updateForum(force);
}

void Siilihai::cancelClicked() {
    foreach(ParserEngine* engine, engines.values())
        engine->cancelOperation();
}

void Siilihai::reportClicked(ForumSubscription* forum) {
    if (forum) {
        ForumParser parserToReport = pdb.getParser(forum->parser());
        ReportParser *rpt = new ReportParser(mainWin, forum->parser(),
                                             parserToReport.parser_name);
        connect(rpt, SIGNAL(parserReport(ParserReport)), &protocol,
                SLOT(sendParserReport(ParserReport)));
        rpt->exec();
    }
}

void Siilihai::statusChanged(ForumSubscription* forum, bool reloading, float progress) {
    updateState();
}

void Siilihai::showUnsubscribeForum(ForumSubscription* fs) {
    if (fs) {
        QMessageBox msgBox(mainWin);
        msgBox.setText("Really unsubscribe from forum?");
        msgBox.setInformativeText(fs->name());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes) {
            protocol.subscribeForum(fs, true);
            fdb.deleteForum(fs);
            pdb.deleteParser(fs->parser());
        }
    }
}

void Siilihai::launchParserMaker() {
#ifndef Q_WS_HILDON
    if (!parserMaker) {
        parserMaker = new ParserMaker(mainWin, pdb, settings, protocol);
        connect(parserMaker, SIGNAL(destroyed()), this,
                SLOT(parserMakerClosed()));
        connect(parserMaker, SIGNAL(parserSaved(ForumParser)), this,
                SLOT(updateForumParser(ForumParser)));

    } else {
        parserMaker->showNormal();
    }
#endif
}

void Siilihai::parserMakerClosed() {
#ifndef Q_WS_HILDON
    if (parserMaker)
        disconnect(parserMaker);
#endif
    parserMaker->deleteLater();
    parserMaker = 0;
}

void Siilihai::sendParserReportFinished(bool success) {
    connect(&protocol, SIGNAL(sendParserReportFinished(bool)), this,
            SLOT(sendParserReportFinished(bool)));

    if (!success) {
        errorDialog("Sending report failed - please try again.");
    } else {
        errorDialog("Thanks for your report");
    }
}
