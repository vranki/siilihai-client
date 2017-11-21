#include "siilihai.h"
#include <siilihai/parser/parsermanager.h>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/parser/parserreport.h>
#include <siilihai/usersettings.h>

#include "threadlistwidget.h"
#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "groupsubscriptiondialog.h"
#include "forumlistwidget.h"
#include "reportparser.h"
#include "../common/credentialsdialog.h"



Siilihai::Siilihai() : ClientLogic()
  , loginWizard(nullptr)
  , mainWin(nullptr)
  , parserMaker(nullptr)
{
    connect(&m_protocol, &SiilihaiProtocol::sendParserReportFinished, this, &Siilihai::sendParserReportFinished);
    connect(this, &ClientLogic::showLoginWizard, this, &Siilihai::showLoginWizardSlot);
    connect(this, &ClientLogic::currentCredentialsRequestChanged, this, &Siilihai::showCredentialsDialogSlot);
    connect(this, &ClientLogic::groupListChanged, this, &Siilihai::showGroupSubscriptionDialog);
}

Siilihai::~Siilihai() {
    if (mainWin) mainWin->deleteLater();
    mainWin = nullptr;
}

void Siilihai::setState(SiilihaiState newState) {
    ClientLogic::setState(newState);

    if(mainWin) mainWin->setOffline(newState==SH_OFFLINE);
}

void Siilihai::showStatusMessage(QString message) {
    if(mainWin) mainWin->showMessage(message);
}

void Siilihai::subscribeForum() {
    if(state() != SH_READY) return;

    SubscribeWizard *subscribeWizard = new SubscribeWizard(mainWin, subscriptionManagement());
    subscribeWizard->setModal(false);
}

void Siilihai::showMainWindow() {
    mainWin = new MainWindow(m_forumDatabase, m_settings);

    connect(mainWin, SIGNAL(subscribeForum()), this, SLOT(subscribeForum()));
    connect(mainWin, SIGNAL(unsubscribeForum(ForumSubscription*)), this, SLOT(showUnsubscribeForum(ForumSubscription*)));
    connect(mainWin, SIGNAL(updateClicked()), this, SLOT(updateClicked()));
    connect(mainWin, SIGNAL(updateClicked(ForumSubscription*,bool)), this, SLOT(updateClicked(ForumSubscription*,bool)));
    connect(mainWin, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
    connect(mainWin, SIGNAL(groupSubscriptions(ForumSubscription*)), this, SLOT(showGroupSubscriptionDialog(ForumSubscription*)));
    connect(mainWin, SIGNAL(reportClicked(ForumSubscription*)), this, SLOT(reportClicked(ForumSubscription*)));
    connect(mainWin, SIGNAL(launchParserMaker()), this, SLOT(launchParserMaker()));
    connect(mainWin, &MainWindow::offlineModeSet, this, &ClientLogic::setOffline);
    connect(mainWin, SIGNAL(haltRequest()), this, SLOT(haltSiilihai()));
    connect(mainWin, SIGNAL(settingsChanged(bool)), this, SLOT(settingsChanged(bool)));
    connect(mainWin, SIGNAL(moreMessagesRequested(ForumThread*)), this, SLOT(moreMessagesRequested(ForumThread*)));
    connect(mainWin, SIGNAL(unsubscribeGroup(ForumGroup*)), this, SLOT(unsubscribeGroup(ForumGroup*)));
    connect(mainWin, SIGNAL(forumUpdateNeeded(ForumSubscription*)), this, SLOT(forumUpdateNeeded(ForumSubscription*)));
    connect(mainWin, SIGNAL(updateThread(ForumThread*, bool)), this, SLOT(updateThread(ForumThread*, bool)));
    connect(mainWin, SIGNAL(unregisterSiilihai()), this, SLOT(unregisterSiilihai()));
    connect(mainWin, SIGNAL(groupUnselected(ForumGroup*)), &m_syncmaster, SLOT(endSyncSingleGroup(ForumGroup *)));
    connect(mainWin, SIGNAL(startSyncClicked()), &m_syncmaster, SLOT(startSync()));
    connect(mainWin, SIGNAL(endSyncClicked()), &m_syncmaster, SLOT(endSync()));
    connect(mainWin, SIGNAL(updateAllParsers()), this, SLOT(updateAllParsers()));
    mainWin->setOffline(offline());
    mainWin->show();
    QApplication::setQuitOnLastWindowClosed(true);
}

void Siilihai::closeUi() {
    mainWin->deleteLater();
    mainWin = nullptr;
    QCoreApplication::quit();
}

// @todo dialog queue!
void Siilihai::errorDialog(QString message) {
    if(message.isEmpty()) return;
    qDebug() << Q_FUNC_INFO << message;
    QMessageBox* msgBox = new QMessageBox(mainWin);
    msgBox->setModal(false);
    msgBox->setText(message);
    msgBox->setWindowTitle("Siilihai error");
    connect(msgBox, SIGNAL(accepted()), msgBox, SLOT(deleteLater()));
    msgBox->open();
}

void Siilihai::showGroupSubscriptionDialog(ForumSubscription* forum) {
    if(state() != SH_READY) return;
    Q_ASSERT(forum);
    GroupSubscriptionDialog *groupSubscriptionDialog = new GroupSubscriptionDialog(mainWin);
    groupSubscriptionDialog->setModal(false);
    groupSubscriptionDialog->setForum(&m_forumDatabase, forum);
    connect(groupSubscriptionDialog, SIGNAL(updateGroupSubscriptions(ForumSubscription*)), this, SLOT(updateGroupSubscriptions(ForumSubscription*)));
    groupSubscriptionDialog->show();
}

void Siilihai::reportClicked(ForumSubscription* forum) {
    if(state() != SH_READY) return;
    if (forum) {
        if(forum->provider() == ForumSubscription::FP_PARSER) {
            ForumParser *parserToReport = qobject_cast<ForumSubscriptionParsed*>(forum)->parserEngine()->parser();
            ReportParser *rpt = new ReportParser(mainWin, parserToReport->id(), parserToReport->name());
            connect(rpt, SIGNAL(parserReport(ParserReport*)), &m_protocol, SLOT(sendParserReport(ParserReport*)));
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
    if (!parserMaker) {
        parserMaker = new ParserMaker(mainWin, m_parserManager, *m_settings, m_protocol);
        connect(parserMaker, SIGNAL(destroyed()), this, SLOT(parserMakerClosed()));
        connect(parserMaker, SIGNAL(parserSaved(ForumParser*)), m_parserManager, SLOT(storeOrUpdateParser(ForumParser*)));
    } else {
        parserMaker->showNormal();
    }
}

void Siilihai::parserMakerClosed() {
    if (parserMaker) {
        disconnect(parserMaker);
        parserMaker->deleteLater();
        parserMaker = nullptr;
    }
}

void Siilihai::sendParserReportFinished(bool success) {
    errorDialog(success ? "Thanks for your report" : "Sending report failed. Please check network connection.");
}

void Siilihai::showLoginWizardSlot() {
    loginWizard = new LoginWizard(mainWin, m_protocol, *m_settings);
    connect(loginWizard, &LoginWizard::finished, this, &ClientLogic::loginWizardFinished);
}

void Siilihai::showCredentialsDialogSlot() {
    if(currentCredentialsRequest()) {
        CredentialsDialog *creds = new CredentialsDialog(mainWin, currentCredentialsRequest());
        creds->show();
    }
}
