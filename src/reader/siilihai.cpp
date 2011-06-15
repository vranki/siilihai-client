#include "siilihai.h"

Siilihai::Siilihai(int& argc, char** argv) : QApplication(argc, argv),
    fdb(this), pdb(this), syncmaster(this, fdb, protocol)
  #ifdef STORE_FILES_IN_APP_DIR
  ,settings(QDir::currentPath() + "/siilihai_settings.ini", QSettings::IniFormat, this)
  #endif
{
    loginWizard = 0;
    mainWin = 0;
    parserMaker = 0;
    progressBar = 0;
    groupSubscriptionDialog = 0;
    subscribeWizard = 0;
    endSyncDone = false;
    firstRun = true;
    srand ( time(NULL) );
}

Siilihai::~Siilihai() {
    if (mainWin)
        mainWin->deleteLater();
    mainWin = 0;
}

void Siilihai::launchSiilihai() {
    currentState = state_started;
    mainWin = new MainWindow(pdb, fdb, &settings);
    db = QSqlDatabase::addDatabase("QSQLITE");
#ifdef STORE_FILES_IN_APP_DIR
    qDebug() << Q_FUNC_INFO << "Loading settings & db from app directory (=Windows build)";
    db.setDatabaseName(QDir::currentPath() + DATABASE_FILE);
#else
    db.setDatabaseName(QDir::homePath() + DATABASE_FILE);
#endif
    if (!db.open()) {
        QMessageBox msgBox(mainWin);
        msgBox.setText("Error: Unable to open database.");
        msgBox.exec();
        haltSiilihai();
        return;
    }
    firstRun = settings.value("first_run", true).toBool();

    settings.setValue("first_run", false);
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
    settingsChanged(false);
    connect(&syncmaster, SIGNAL(syncFinished(bool, QString)), this, SLOT(syncFinished(bool, QString)));
    connect(&syncmaster, SIGNAL(syncProgress(float)), this, SLOT(syncProgress(float)));
    protocol.setBaseURL(baseUrl);

    int mySchema = settings.value("forum_database_schema", 0).toInt();
    if(!firstRun && fdb.schemaVersion() != mySchema) {
        errorDialog("The database schema has been changed. Your forum database will be reset. Sorry. ");
        fdb.resetDatabase();
    }
    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), this, SLOT(subscriptionFound(ForumSubscription*)));
    connect(&fdb, SIGNAL(databaseStored()), this, SLOT(databaseStored()), Qt::QueuedConnection);
    connect(&protocol, SIGNAL(getParserFinished(ForumParser)), this, SLOT(updateForumParser(ForumParser)));
    connect(&protocol, SIGNAL(userSettingsReceived(bool,UserSettings*)), this,
            SLOT(userSettingsReceived(bool,UserSettings*)));
    if(fdb.openDatabase(&db)) {
        settings.setValue("forum_database_schema", fdb.schemaVersion());
    } else {
        errorDialog("Error opening Siilihai's database!\n"
                    "See console for details. Sorry.\n\n"
                    "Check that you're not running aother copy of Siilihai.\n"
                    "You can delete ~/.siilihai.db to reset database.");
        haltSiilihai();
        return;
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

void Siilihai::changeState(siilihai_states newState) {
    siilihai_states previousState = currentState;
    currentState = newState;

    if(newState==state_offline) {
        qDebug() << Q_FUNC_INFO << "Offline";
        Q_ASSERT(previousState==state_login || previousState==state_startsyncing
                 || previousState==state_updating_parsers
                 || previousState==state_ready || previousState==state_started);
        if(previousState==state_startsyncing)
            syncmaster.cancel();
        if(previousState==state_updating_parsers)
            parsersToUpdateLeft.clear();

        mainWin->setReaderReady(true, true);
        if(progressBar) { // exists if canceling dureing login process
            progressBar->cancel();
            progressBar->deleteLater();
            progressBar = 0;
        }
    } else if(newState==state_login) {
        qDebug() << Q_FUNC_INFO << "Login";
        if(!progressBar) {
            progressBar = new QProgressDialog("Logging in", "Cancel", 0, 100,
                                              mainWin);
            progressBar->setWindowModality(Qt::WindowModal);
            progressBar->setValue(0);
            connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
        }
        progressBar->setLabelText("Logging in..");
        progressBar->setValue(5);
    } else if(newState==state_startsyncing) {
        qDebug() << Q_FUNC_INFO << "Startsync";
        if(usettings.syncEnabled())
            syncmaster.startSync();
        Q_ASSERT(progressBar);
        progressBar->setValue(10);
        if(firstRun) {
            progressBar->setLabelText("Downloading message read status.\nOn first run this may take a few minutes.");
        } else {
            progressBar->setLabelText("Downloading message read status");
        }
    } else if(newState==state_endsync) {
        qDebug() << Q_FUNC_INFO << "Endsync";
        mainWin->setReaderReady(false, false);
        Q_ASSERT(!progressBar);
        progressBar = new QProgressDialog("Synchronizing with server", "Cancel", 0, 100, mainWin);
        progressBar->setModal(true);
        progressBar->setValue(0);
        connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
    } else if(newState==state_storedb) {
        qDebug() << Q_FUNC_INFO << "Storedb";
        if(!progressBar) {
            progressBar = new QProgressDialog("Stroring changes", "Cancel", 0, 100, mainWin);
            progressBar->setValue(0);
            connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
        }
        progressBar->setLabelText("Storing changes to local database");
        progressBar->setModal(true);
        progressBar->setValue(75);
        fdb.storeDatabase();
        mainWin->hide(); // Is this good?
    } else if(newState==state_updating_parsers) {
        qDebug() << Q_FUNC_INFO << "Update parsers";
        if(parsersToUpdateLeft.isEmpty()) {
            changeState(state_ready);
            return;
        }
        Q_ASSERT(progressBar);
        progressBar->setValue(80);
        progressBar->setLabelText("Updating parser definitions");
        protocol.getParser(parsersToUpdateLeft.takeFirst()->parser());
    } else if(newState==state_ready) {
        qDebug() << Q_FUNC_INFO << "Ready";
        Q_ASSERT(previousState==state_updating_parsers);
        Q_ASSERT(progressBar);
        progressBar->cancel();
        progressBar->deleteLater();
        progressBar = 0;
        while(!subscriptionsNeedingCredentials.isEmpty()) {
            ForumSubscription *sub = subscriptionsNeedingCredentials.takeFirst();
            QAuthenticator authenticator;
            CredentialsDialog *creds = new CredentialsDialog(mainWin, sub, &authenticator, NULL);
            creds->setModal(true);
            creds->exec();
            if(authenticator.user().length() > 0) {
                sub->setUsername(authenticator.user());
                sub->setPassword(authenticator.password());
            } else {
                sub->setAuthenticated(false);
            }
            protocol.subscribeForum(sub);
        }
        mainWin->setReaderReady(true, false);
        if (fdb.isEmpty()) { // Display subscribe dialog if none subscribed
            subscribeForum();
        }

        if (settings.value("preferences/update_automatically", true).toBool())
            updateClicked();
    }
}

void Siilihai::tryLogin() {
    Q_ASSERT(currentState==state_started || currentState==state_offline);
    changeState(state_login);

    connect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this,
            SLOT(loginFinished(bool, QString,bool)));
    protocol.login(settings.value("account/username", "").toString(),
                   settings.value("account/password", "").toString());
}

void Siilihai::haltSiilihai() {
    cancelClicked();
    foreach(ParserEngine *engine, engines.values())
        engine->deleteLater();
    engines.clear();
    if(usettings.syncEnabled() && !endSyncDone && currentState == state_ready) {
        qDebug() << "Sync enabled - running end sync";
        changeState(state_endsync);
        syncmaster.endSync();
    } else {
        if(currentState != state_storedb && fdb.isStored()) {
            changeState(state_storedb);
        } else {
            qDebug() << "All done - quitting";
            settings.sync();
            if(progressBar)
                progressBar->deleteLater();
            mainWin->deleteLater();
            mainWin = 0;
            progressBar = 0;
            fdb.storeDatabase();
            quit();
        }
    }
}

void Siilihai::syncFinished(bool success, QString message){
    qDebug() << Q_FUNC_INFO << success;
    if (!success) {
        errorDialog(QString("Syncing status to server failed.\n\n%1").arg(message));
    }
    if(currentState == state_startsyncing) {
        changeState(state_updating_parsers);
    } else if(currentState == state_endsync) {
        endSyncDone = true;
        haltSiilihai();
    }
}

void Siilihai::offlineModeSet(bool newOffline) {
    qDebug() << Q_FUNC_INFO << newOffline;
    if(newOffline && currentState == state_ready) {
        changeState(state_offline);
    } else if(!newOffline && currentState == state_offline) {
        tryLogin();
    }
}

void Siilihai::loginFinished(bool success, QString motd, bool sync) {
    qDebug() << Q_FUNC_INFO << success;
    disconnect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this,
               SLOT(loginFinished(bool, QString,bool)));
    if(!progressBar) { // Make sure this exists. If  user logs in first time, it might not!
        progressBar = new QProgressDialog("Logged in", "Cancel", 0, 100, mainWin);
        progressBar->setWindowModality(Qt::WindowModal);
        progressBar->setValue(0);
        connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
    }
    if (success) {
        connect(&protocol, SIGNAL(listSubscriptionsFinished(QList<int>)), this,
                SLOT(listSubscriptionsFinished(QList<int>)));
        connect(&protocol, SIGNAL(sendParserReportFinished(bool)), this,
                SLOT(sendParserReportFinished(bool)));
        connect(&protocol, SIGNAL(subscribeForumFinished(ForumSubscription*, bool)), this,
                SLOT(subscribeForumFinished(ForumSubscription*,bool)));
        usettings.setSyncEnabled(sync);
        settings.setValue("preferences/sync_enabled", usettings.syncEnabled());
        settings.sync();
        progressBar->setValue(30);
        if(usettings.syncEnabled()) {
            changeState(state_startsyncing);
        } else {
            changeState(state_updating_parsers);
        }
    } else {
        progressBar->cancel();
        progressBar->deleteLater();
        progressBar = 0;

        QMessageBox msgBox(mainWin);
        msgBox.setModal(true);
        if (motd.length() > 0) {
            msgBox.setText(motd);
        } else {
            msgBox.setText("Error: Login failed. Check your username, password and network connection.\nWorking offline.");
        }
        msgBox.exec();
        changeState(state_offline);
    }
}

// @todo never called?
void Siilihai::listSubscriptionsFinished(QList<int> serversSubscriptions) {
    qDebug() << Q_FUNC_INFO << "count of subscribed forums " << serversSubscriptions.size();
    disconnect(&protocol, SIGNAL(listSubscriptionsFinished(QList<int>)), this,
               SLOT(listSubscriptionsFinished(QList<int>)));
    if (progressBar)
        progressBar->setValue(50);

    QList<ForumSubscription*> unsubscribedForums;
    foreach(ForumSubscription* sub, fdb.values()) {
        bool found = false;
        foreach(int serverSubscriptionId, serversSubscriptions) {
            qDebug() << "Server says: subscribed to " << serverSubscriptionId;
            if (serverSubscriptionId == sub->parser())
                found = true;
        }
        if (!found) {
            qDebug() << "Server says not subscribed to " << sub->toString();
            unsubscribedForums.append(sub);
        }
    }
    foreach (ForumSubscription *sub, unsubscribedForums) {
        qDebug() << "Deleting forum " << sub->toString() << "as server says it's not subscribed";
        fdb.deleteSubscription(sub);
        pdb.deleteParser(sub->parser());
    }

}

// Stores the parser if subscribed to it and updates the engine
void Siilihai::updateForumParser(ForumParser parser) {
    qDebug() << Q_FUNC_INFO << parser.id;
    if (parser.isSane()) {
        foreach(ForumSubscription *sub, engines.keys()) { // Find subscription that uses parser
            if (sub->parser() == parser.id) {
                pdb.storeParser(parser);
                engines[sub]->setParser(parser);
                emit statusChanged(sub, false, -1);
                if (parsersToUpdateLeft.size() < 2) {
                    if (progressBar)
                        progressBar->setValue(90);
                }
            }
        }
    }
    if(currentState == state_updating_parsers) {
        if(parsersToUpdateLeft.isEmpty()) {
            changeState(state_ready);
        } else {
            protocol.getParser(parsersToUpdateLeft.takeFirst()->parser());
            if (progressBar)
                progressBar->setValue(85);
        }
    }
}

void Siilihai::subscribeForum() {
    subscribeWizard = new SubscribeWizard(mainWin, protocol, baseUrl, settings);
    subscribeWizard->setModal(true);
    connect(subscribeWizard,
            SIGNAL(forumAdded(ForumParser, ForumSubscription*)), this,
            SLOT(forumAdded(ForumParser, ForumSubscription*)));
}


void Siilihai::loginWizardFinished() {
    loginWizard->deleteLater();
    loginWizard = 0;
    if (settings.value("account/username", "").toString().length() == 0) {
        qDebug() << "Settings wizard failed, quitting.";
        haltSiilihai();
    } else {
        launchMainWindow();
        settingsChanged(false);
        loginFinished(true, QString(), usettings.syncEnabled());
        if(firstRun && settings.value("account/registered_here", false).toBool())
            subscribeForum();
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
    connect(mainWin, SIGNAL(launchParserMaker()), this,
            SLOT(launchParserMaker()));
    connect(mainWin, SIGNAL(offlineModeSet(bool)), this,
            SLOT(offlineModeSet(bool)));
    connect(mainWin, SIGNAL(haltRequest()), this,
            SLOT(haltSiilihai()));
    connect(mainWin, SIGNAL(settingsChanged(bool)), this, SLOT(settingsChanged(bool)));
    connect(mainWin, SIGNAL(moreMessagesRequested(ForumThread*)), this, SLOT(moreMessagesRequested(ForumThread*)));
    connect(mainWin, SIGNAL(unsubscribeGroup(ForumGroup*)), this, SLOT(unsubscribeGroup(ForumGroup*)));
    connect(mainWin, SIGNAL(forumUpdateNeeded(ForumSubscription*)), this, SLOT(forumUpdateNeeded(ForumSubscription*)));
    connect(mainWin->threadList(), SIGNAL(updateThread(ForumThread*, bool)), this, SLOT(updateThread(ForumThread*, bool)));
    connect(mainWin, SIGNAL(unregisterSiilihai()), this, SLOT(unregisterSiilihai()));
    mainWin->setReaderReady(false, currentState==state_offline);
    mainWin->show();
    setQuitOnLastWindowClosed(true);
}

void Siilihai::forumAdded(ForumParser fp, ForumSubscription *fs) {
    qDebug() << Q_FUNC_INFO << fp.toString() << fs->toString();
    if(subscribeWizard) {
        subscribeWizard->deleteLater();
        subscribeWizard = 0;
    }
    ForumSubscription *newFs = new ForumSubscription(&fdb, false);
    newFs->copyFrom(fs);
    fs = 0;
    if(!fdb.addSubscription(newFs) || !pdb.storeParser(fp)) {
        QMessageBox msgBox(mainWin);
        msgBox.setText("Error: Unable to subscribe to forum. Are you already subscribed?");
        msgBox.exec();
    } else {
        protocol.subscribeForum(newFs);
        ParserEngine *newEngine = engines.value(newFs);
        newEngine->setParser(fp);
        newEngine->updateGroupList();
    }
}

void Siilihai::subscriptionFound(ForumSubscription *sub) {
    connect(sub, SIGNAL(destroyed(QObject*)), this, SLOT(subscriptionDeleted(QObject*)));

    ParserEngine *pe = new ParserEngine(&fdb, this);
    ForumParser parser = pdb.getParser(sub->parser());
    pe->setParser(parser);
    pe->setSubscription(sub);
    engines[sub] = pe;
    //    mainWin->forumList()->addSubscription(sub);
    connect(pe, SIGNAL(groupListChanged(ForumSubscription*)), this,
            SLOT(showSubscribeGroup(ForumSubscription*)));
    connect(pe, SIGNAL(forumUpdated(ForumSubscription*)), this, SLOT(forumUpdated(ForumSubscription*)));
        /*
    connect(pe, SIGNAL(statusChanged(ForumSubscription*, bool, float)), this,
            SLOT(statusChanged(ForumSubscription*, bool, float)));

    connect(pe, SIGNAL(statusChanged(ForumSubscription*, bool, float)), mainWin,
            SLOT(setForumStatus(ForumSubscription*, bool, float)));
            */
    connect(pe, SIGNAL(updateFailure(ForumSubscription*, QString)), this,
            SLOT(updateFailure(ForumSubscription*, QString)));
    connect(pe, SIGNAL(getAuthentication(ForumSubscription*, QAuthenticator*)),
            this, SLOT(getAuthentication(ForumSubscription*,QAuthenticator*)));
    connect(pe, SIGNAL(loginFinished(ForumSubscription*,bool)), this, SLOT(forumLoginFinished(ForumSubscription*,bool)));
    parsersToUpdateLeft.append(sub);
    if(sub->authenticated() && sub->username().length()==0) {
        subscriptionsNeedingCredentials.append(sub);
    }
}

void Siilihai::subscriptionDeleted(QObject* subobj) {
    qDebug() << Q_FUNC_INFO;
    ForumSubscription *sub = static_cast<ForumSubscription*> (subobj);
    if(!engines.contains(sub)) return; // Possible when quitting
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
    qDebug() << Q_FUNC_INFO << forum->toString();
    // @todo stupid logic to prevent dialog from synced groups.
    if (currentState == state_ready) {
        groupSubscriptionDialog = new GroupSubscriptionDialog(mainWin);
        groupSubscriptionDialog->setModal(false);
        groupSubscriptionDialog->setForum(&fdb, forum);
        connect(groupSubscriptionDialog, SIGNAL(finished(int)), this,
                SLOT(subscribeGroupDialogFinished()));
        groupSubscriptionDialog->exec();
    }
}

void Siilihai::subscribeGroupDialogFinished() {
    if (currentState == state_ready) {
        protocol.updateGroupSubscriptions(groupSubscriptionDialog->subscription());
        engines.value(groupSubscriptionDialog->subscription())->updateForum();
    }
    groupSubscriptionDialog->deleteLater();
    groupSubscriptionDialog = 0;
}

void Siilihai::forumUpdated(ForumSubscription* forum) {
    subscriptionsToUpdateLeft.removeOne(forum);
    if(!subscriptionsToUpdateLeft.isEmpty()) {
        engines.value(subscriptionsToUpdateLeft.first())->updateForum();
        subscriptionsToUpdateLeft.removeOne(subscriptionsToUpdateLeft.first());
    }
}

void Siilihai::updateClicked() {
    int parsersUpdating = 0;
    foreach(ParserEngine* engine, engines.values()) {
        if(parsersUpdating <= MAX_CONCURRENT_UPDATES) {
            engine->updateForum();
            parsersUpdating++;
        } else {
            subscriptionsToUpdateLeft.append(engine->subscription());
        }
    }
}

void Siilihai::updateClicked(ForumSubscription* sub , bool force) {
    Q_ASSERT(engines.contains(sub));
    engines[sub]->updateForum(force);
}

void Siilihai::updateThread(ForumThread* thread, bool force) {
    ForumSubscription *sub = thread->group()->subscription();
    Q_ASSERT(sub);
    Q_ASSERT(engines.contains(sub));
    engines[sub]->updateThread(thread, force);
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
}

void Siilihai::showUnsubscribeForum(ForumSubscription* fs) {
    if (fs) {
        QMessageBox msgBox(mainWin);
        msgBox.setText("Really unsubscribe from forum?");
        msgBox.setInformativeText(fs->alias());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes) {
            protocol.subscribeForum(fs, true);
            fdb.deleteSubscription(fs);
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
    qDebug() << Q_FUNC_INFO << success;
    if (!success) {
        errorDialog("Sending report failed. Please check network connection.");
    } else {
        errorDialog("Thanks for your report");
    }
}

void Siilihai::subscribeForumFinished(ForumSubscription *sub, bool success) {
    qDebug() << Q_FUNC_INFO << success;
    if (!success) {
        errorDialog("Subscribing to forum failed. Please check network connection.");
        if(fdb.value(sub->parser()))
            fdb.deleteSubscription(sub);
    }
}

void Siilihai::userSettingsReceived(bool success, UserSettings *newSettings) {
    qDebug() << Q_FUNC_INFO << success;
    if (!success) {
        errorDialog("Getting settings failed. Please check network connection.");
    } else {
        usettings.setSyncEnabled(newSettings->syncEnabled());
        settings.setValue("preferences/sync_enabled", usettings.syncEnabled());
        settings.sync();
        settingsChanged(false);
    }
}

void Siilihai::settingsChanged(bool byUser) {
    usettings.setSyncEnabled(settings.value("preferences/sync_enabled", false).toBool());
    if(byUser) {
        protocol.setUserSettings(&usettings);
    }
}

void Siilihai::cancelProgress() {
    qDebug() << Q_FUNC_INFO;
    if(currentState==state_login) {
        loginFinished(false,QString::null,false);
    } else if(currentState==state_updating_parsers || currentState==state_startsyncing) {
        changeState(state_offline);
    } else if(currentState==state_endsync) {
        haltSiilihai();
    } else if(currentState==state_storedb) { // Not allowed
    } else {
        Q_ASSERT(false);
    }
}

void Siilihai::getAuthentication(ForumSubscription *fsub, QAuthenticator *authenticator) {
    bool failed = false;
    QString gname = QString().number(fsub->parser());
    settings.beginGroup("authentication");
    if(settings.contains(QString("%1/username").arg(gname))) {
        authenticator->setUser(settings.value(QString("%1/username").arg(gname)).toString());
        authenticator->setPassword(settings.value(QString("%1/password").arg(gname)).toString());
        if(settings.value(QString("authentication/%1/failed").arg(gname)).toString() == "true") failed = true;
        qDebug() << Q_FUNC_INFO << "Failed: " << settings.value(QString("%1/failed").arg(gname)).toString();
    }
    settings.endGroup();
    if(authenticator->user().isNull() || failed) {
        CredentialsDialog *creds = new CredentialsDialog(mainWin, fsub, authenticator, &settings);
        creds->setModal(true);
        creds->exec();
    }
}
void Siilihai::updateFailure(ForumSubscription* sub, QString msg) {
    settings.setValue(QString("authentication/%1/failed").arg(sub->parser()), "true");
    errorDialog(sub->alias() + "\n" + msg);
}

void Siilihai::moreMessagesRequested(ForumThread* thread) {
    Q_ASSERT(thread);
    ParserEngine *engine = engines[thread->group()->subscription()];
    Q_ASSERT(engine);
    if(engine->state() == ParserEngine::PES_UPDATING) return;

    thread->setGetMessagesCount(thread->getMessagesCount() +
                                settings.value("preferences/show_more_count", 30).toInt());
    thread->commitChanges();
    engine->updateThread(thread);
}

void Siilihai::unsubscribeGroup(ForumGroup *group) {
    group->setSubscribed(false);
    group->commitChanges();
    protocol.updateGroupSubscriptions(group->subscription());
}

void Siilihai::forumLoginFinished(ForumSubscription *sub, bool success) {
    qDebug() << Q_FUNC_INFO << sub->toString() << success;
    if(!success)
        QMessageBox::critical(mainWin, "Error",
                              QString("Login to %1 failed. Please check credentials.").arg(sub->alias()),
                              QMessageBox::Ok);
}

void Siilihai::forumUpdateNeeded(ForumSubscription *fs) {
    qDebug() << Q_FUNC_INFO;
    protocol.subscribeForum(fs);
}

void Siilihai::syncProgress(float progress) {
    if(progressBar) {
        if(currentState==state_startsyncing)
            progressBar->setValue(10 + progress * 70);
        else if(currentState==state_startsyncing)
            progressBar->setValue(progress * 100);
    }
}

void Siilihai::unregisterSiilihai() {
    cancelClicked();
    fdb.resetDatabase();
    settings.remove("account/username");
    settings.remove("account/password");
    settings.remove("first_run");
    fdb.storeDatabase();
    usettings.setSyncEnabled(false);
    QMessageBox::information(mainWin, "Unregister successful", "Siilihai has been unregistered and will now quit.");
    haltSiilihai();
}

void Siilihai::databaseStored() {
    haltSiilihai();
}
