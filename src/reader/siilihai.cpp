#include "siilihai.h"
#include <siilihai/parsermanager.h>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/parserreport.h>
#include <siilihai/usersettings.h>
#include <QDesktopServices>
#include "threadlistwidget.h"
#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "groupsubscriptiondialog.h"
#include "forumlistwidget.h"
#include "reportparser.h"
#include "../common/credentialsdialog.h"

Siilihai::Siilihai() : ClientLogic() {
    loginWizard = 0;
    mainWin = 0;
    parserMaker = 0;
    progressBar = 0;
    groupSubscriptionDialog = 0;
}

Siilihai::~Siilihai() {
    if (mainWin)
        mainWin->deleteLater();
    mainWin = 0;
}


void Siilihai::changeState(siilihai_states newState) {
    ClientLogic::changeState(newState);

    if(newState==SH_OFFLINE) {
        mainWin->setReaderReady(true, true);
        if(progressBar) { // exists if canceling dureing login process
            progressBar->cancel();
            progressBar->deleteLater();
            progressBar = 0;
        }
    } else if(newState==SH_LOGIN) {
        if(!progressBar) {
            progressBar = new QProgressDialog("Logging in", "Cancel", 0, 100, mainWin);
            progressBar->setWindowModality(Qt::WindowModal);
            progressBar->setValue(0);
            ClientLogic::connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
        }
        progressBar->setLabelText("Logging in..");
        progressBar->setValue(5);
    } else if(newState==SH_STARTSYNCING) {
        if(progressBar) { // exists if canceling dureing login process
            progressBar->cancel();
            progressBar->deleteLater();
            progressBar = 0;
        }
    } else if(newState==SH_ENDSYNC) {
        mainWin->setReaderReady(false, false);
        Q_ASSERT(!progressBar);
        progressBar = new QProgressDialog("Synchronizing with server", "Cancel", 0, 100, mainWin);
        progressBar->setModal(true);
        progressBar->setValue(0);
        connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
    } else if(newState==SH_STOREDB) {
        if(!progressBar) {
            progressBar = new QProgressDialog("Storing changes", "Cancel", 0, 100, mainWin);
            progressBar->setValue(0);
            connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
        }
        progressBar->setLabelText("Storing changes to local database");
        progressBar->setModal(true);
        progressBar->setValue(75);
    } else if(newState==SH_READY) {
        if(progressBar) {
            progressBar->cancel();
            progressBar->deleteLater();
            progressBar = 0;
        }
        mainWin->setReaderReady(true, false);
    }
}

void Siilihai::loginFinished(bool success, QString motd, bool sync) {
    ClientLogic::loginFinished(success, motd, sync);
    if(!progressBar) { // Make sure this exists. If  user logs in first time, it might not!
        progressBar = new QProgressDialog("Logged in", "Cancel", 0, 100, mainWin);
        progressBar->setWindowModality(Qt::WindowModal);
        progressBar->setValue(0);
        connect(progressBar, SIGNAL(canceled()), this, SLOT(cancelProgress()));
    }
    if (success) {
        progressBar->setValue(30);
    } else {
        progressBar->cancel();
        progressBar->deleteLater();
        progressBar = 0;
    }
}

void Siilihai::subscribeForum() {
    SubscribeWizard *subscribeWizard = new SubscribeWizard(mainWin, protocol, *settings);
    subscribeWizard->setModal(false);
    connect(subscribeWizard, SIGNAL(forumAdded(ForumSubscription*)), this, SLOT(forumAdded(ForumSubscription*)));
}

void Siilihai::showMainWindow() {
    mainWin = new MainWindow(forumDatabase, settings);

    connect(mainWin, SIGNAL(subscribeForum()), this, SLOT(subscribeForum()));
    connect(mainWin, SIGNAL(unsubscribeForum(ForumSubscription*)), this, SLOT(showUnsubscribeForum(ForumSubscription*)));
    connect(mainWin, SIGNAL(updateClicked()), this, SLOT(updateClicked()));
    connect(mainWin, SIGNAL(updateClicked(ForumSubscription*,bool)), this, SLOT(updateClicked(ForumSubscription*,bool)));
    connect(mainWin, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    connect(mainWin, SIGNAL(groupSubscriptions(ForumSubscription*)), this, SLOT(showSubscribeGroup(ForumSubscription*)));
    connect(mainWin, SIGNAL(reportClicked(ForumSubscription*)), this, SLOT(reportClicked(ForumSubscription*)));
    connect(mainWin, SIGNAL(launchParserMaker()), this, SLOT(launchParserMaker()));
    connect(mainWin, SIGNAL(offlineModeSet(bool)), this, SLOT(offlineModeSet(bool)));
    connect(mainWin, SIGNAL(haltRequest()), this, SLOT(haltSiilihai()));
    connect(mainWin, SIGNAL(settingsChanged(bool)), this, SLOT(settingsChanged(bool)));
    connect(mainWin, SIGNAL(moreMessagesRequested(ForumThread*)), this, SLOT(moreMessagesRequested(ForumThread*)));
    connect(mainWin, SIGNAL(unsubscribeGroup(ForumGroup*)), this, SLOT(unsubscribeGroup(ForumGroup*)));
    connect(mainWin, SIGNAL(forumUpdateNeeded(ForumSubscription*)), this, SLOT(forumUpdateNeeded(ForumSubscription*)));
    connect(mainWin->threadList(), SIGNAL(updateThread(ForumThread*, bool)), this, SLOT(updateThread(ForumThread*, bool)));
    connect(mainWin, SIGNAL(unregisterSiilihai()), this, SLOT(unregisterSiilihai()));
    connect(&syncmaster, SIGNAL(syncProgress(float, QString)), mainWin, SLOT(syncProgress(float, QString)));
    connect(&forumDatabase, SIGNAL(subscriptionFound(ForumSubscription*)), mainWin->forumList(), SLOT(addSubscription(ForumSubscription*)));

    foreach(ForumSubscription *sub, forumDatabase.values())
        mainWin->forumList()->addSubscription(sub);

    mainWin->setReaderReady(false, currentState==SH_OFFLINE);
    mainWin->show();
    QApplication::setQuitOnLastWindowClosed(true);
}

void Siilihai::closeUi() {
    if(progressBar)
        progressBar->deleteLater();
    mainWin->deleteLater();
    mainWin = 0;
    progressBar = 0;
    QCoreApplication::quit();
}
void Siilihai::settingsChanged(bool byUser) {
    ClientLogic::settingsChanged(byUser);
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
    if (currentState == SH_READY) {
        groupSubscriptionDialog = new GroupSubscriptionDialog(mainWin);
        groupSubscriptionDialog->setModal(false);
        groupSubscriptionDialog->setForum(&forumDatabase, forum);
        connect(groupSubscriptionDialog, SIGNAL(finished(int)), this, SLOT(subscribeGroupDialogFinished()));
        groupSubscriptionDialog->exec();
    }
}


void Siilihai::reportClicked(ForumSubscription* forum) {
    if (forum) {
        ForumParser *parserToReport = forum->parserEngine()->parser();
        ReportParser *rpt = new ReportParser(mainWin, forum->parser(), parserToReport->parser_name);
        connect(rpt, SIGNAL(parserReport(ParserReport*)), &protocol, SLOT(sendParserReport(ParserReport*)));
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
            unsubscribeForum(fs);

        }
    }
}

void Siilihai::launchParserMaker() {
#ifndef Q_WS_HILDON
    if (!parserMaker) {
        parserMaker = new ParserMaker(mainWin, parserManager, *settings, protocol);
        connect(parserMaker, SIGNAL(destroyed()), this, SLOT(parserMakerClosed()));
        connect(parserMaker, SIGNAL(parserSaved(ForumParser*)), parserManager, SLOT(storeOrUpdateParser(ForumParser*)));
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

void Siilihai::cancelProgress() {
    qDebug() << Q_FUNC_INFO;
    if(currentState==SH_LOGIN) {
        loginFinished(false,QString::null,false);
    } else if(currentState==SH_STARTSYNCING) {
        changeState(SH_OFFLINE);
    } else if(currentState==SH_ENDSYNC) {
        haltSiilihai();
    } else if(currentState==SH_STOREDB) { // Not allowed
    } else {
        Q_ASSERT(false);
    }
}

void Siilihai::getAuthentication(ForumSubscription *fsub, QAuthenticator *authenticator) {
    bool failed = false;
    QString gname = QString().number(fsub->parser());
    settings->beginGroup("authentication");
    if(settings->contains(QString("%1/username").arg(gname))) {
        authenticator->setUser(settings->value(QString("%1/username").arg(gname)).toString());
        authenticator->setPassword(settings->value(QString("%1/password").arg(gname)).toString());
        if(settings->value(QString("authentication/%1/failed").arg(gname)).toString() == "true") failed = true;
    }
    settings->endGroup();
    if(authenticator->user().isNull() || failed) {
        CredentialsDialog *creds = new CredentialsDialog(mainWin, fsub, authenticator, settings);
        creds->setModal(true);
        creds->exec();
    }
}




// Caution - engine->subscription() may be null (when deleted)!
void Siilihai::parserEngineStateChanged(ParserEngine *engine, ParserEngine::ParserEngineState newState, ParserEngine::ParserEngineState oldState) {
    ClientLogic::parserEngineStateChanged(engine,  newState, oldState);
    if(newState == ParserEngine::PES_REQUESTING_CREDENTIALS) {
        ForumSubscription *sub = engine->subscription();
        QAuthenticator *authenticator = new QAuthenticator();
        CredentialsDialog *creds = new CredentialsDialog(mainWin, sub, authenticator, settings);
        connect(creds, SIGNAL(credentialsEntered(QAuthenticator*)), engine, SLOT(credentialsEntered(QAuthenticator*)));
        creds->setModal(false);
        creds->show();
    }
}

QString Siilihai::getDataFilePath() {
#ifdef STORE_FILES_IN_APP_DIR
    return applicationDirPath();
#else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}

void Siilihai::showLoginWizard() {
    loginWizard = new LoginWizard(mainWin, protocol, *settings);
    connect(loginWizard, SIGNAL(finished(int)), this, SLOT(loginWizardFinished()));
}

void Siilihai::subscribeGroupDialogFinished() {
    if (currentState == SH_READY && groupSubscriptionDialog->subscription()) {
        updateGroupSubscriptions(groupSubscriptionDialog->subscription());
    }
    groupSubscriptionDialog->deleteLater();
    groupSubscriptionDialog = 0;
}
