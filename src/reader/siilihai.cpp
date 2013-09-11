#include "siilihai.h"
#include <siilihai/parser/parsermanager.h>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/parser/parserreport.h>
#include <siilihai/usersettings.h>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif
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
}

Siilihai::~Siilihai() {
    if (mainWin)
        mainWin->deleteLater();
    mainWin = 0;
}

void Siilihai::changeState(siilihai_states newState) {
    ClientLogic::changeState(newState);

    if(mainWin) mainWin->setOffline(newState==SH_OFFLINE);
}

void Siilihai::showStatusMessage(QString message) {
    if(mainWin) mainWin->showMessage(message);
}

void Siilihai::subscribeForum() {
    if(currentState != SH_READY) return;
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
    connect(mainWin, SIGNAL(updateThread(ForumThread*, bool)), this, SLOT(updateThread(ForumThread*, bool)));
    connect(mainWin, SIGNAL(unregisterSiilihai()), this, SLOT(unregisterSiilihai()));
    connect(mainWin, SIGNAL(groupUnselected(ForumGroup*)), &syncmaster, SLOT(endSyncSingleGroup(ForumGroup *)));
    connect(mainWin, SIGNAL(startSyncClicked()), &syncmaster, SLOT(startSync()));
    connect(mainWin, SIGNAL(endSyncClicked()), &syncmaster, SLOT(endSync()));
    connect(mainWin, SIGNAL(updateAllParsers()), this, SLOT(updateAllParsers()));
    mainWin->setOffline(currentState==SH_OFFLINE);
    mainWin->show();
    QApplication::setQuitOnLastWindowClosed(true);
}

void Siilihai::closeUi() {
    mainWin->deleteLater();
    mainWin = 0;
    QCoreApplication::quit();
}

void Siilihai::errorDialog(QString message) {
    QMessageBox* msgBox = new QMessageBox(mainWin);
    msgBox->setModal(false);
    msgBox->setText(message);
    connect(msgBox, SIGNAL(accepted()), msgBox, SLOT(deleteLater()));
    msgBox->open();
}

void Siilihai::showSubscribeGroup(ForumSubscription* forum) {
    if(currentState != SH_READY) return;
    Q_ASSERT(forum);
    GroupSubscriptionDialog *groupSubscriptionDialog = new GroupSubscriptionDialog(mainWin);
    groupSubscriptionDialog->setModal(false);
    groupSubscriptionDialog->setForum(&forumDatabase, forum);
    connect(groupSubscriptionDialog, SIGNAL(updateGroupSubscriptions(ForumSubscription*)), this, SLOT(updateGroupSubscriptions(ForumSubscription*)));
    groupSubscriptionDialog->show();
}

void Siilihai::reportClicked(ForumSubscription* forum) {
    if(currentState != SH_READY) return;
    if (forum) {
        if(forum->isParsed()) {
            ForumParser *parserToReport = qobject_cast<ForumSubscriptionParsed*>(forum)->parserEngine()->parser();
            ReportParser *rpt = new ReportParser(mainWin, parserToReport->id(), parserToReport->name());
            connect(rpt, SIGNAL(parserReport(ParserReport*)), &protocol, SLOT(sendParserReport(ParserReport*)));
            rpt->exec();
        } else {
            errorDialog(forum->alias() + " does not use a parser");
        }
    }
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
    if (!success) {
        errorDialog("Sending report failed. Please check network connection.");
    } else {
        errorDialog("Thanks for your report");
    }
}

QString Siilihai::getDataFilePath() {
#if QT_VERSION < 0x050000
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
}

void Siilihai::showLoginWizard() {
    loginWizard = new LoginWizard(mainWin, protocol, *settings);
    connect(loginWizard, SIGNAL(finished(int)), this, SLOT(loginWizardFinished()));
}

void Siilihai::loginWizardFinished() {
    ClientLogic::loginWizardFinished();
}

void Siilihai::settingsChanged(bool byUser) {
    ClientLogic::settingsChanged(byUser);
}

void Siilihai::showCredentialsDialog(CredentialsRequest *cr) {
    CredentialsDialog *creds = new CredentialsDialog(mainWin, cr);
    creds->setModal(true);
    creds->exec();
    creds->deleteLater();
}
