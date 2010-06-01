#ifndef SIILIHAI_H_
#define SIILIHAI_H_
#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QtSql>
#include <QDir>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QProgressDialog>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/parserdatabase.h>
#include <siilihai/parserreport.h>
#include <siilihai/parserengine.h>
#include <siilihai/syncmaster.h>
#include <siilihai/usersettings.h>

#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "groupsubscriptiondialog.h"
#include "forumlistwidget.h"
#include "reportparser.h"
#include "../common/credentialsdialog.h"

#ifndef Q_WS_HILDON
#include "../parsermaker/parsermaker.h"
#else
class ParserMaker;
#endif

#define DATABASE_FILE "/.siilihai.db"
#define BASEURL "http://www.siilihai.com/"

// State chart:
//                 ,------>-------.
// started -> login ->startsync -> update parsers -> ready -> endsync -> quit
//              | ^       |              |            |                   ^
//              v |   .-------------<-----------------'                   |
//             offline ------------------------>--------------------------'
//

class Siilihai: public QApplication {
    Q_OBJECT

    enum siilihai_states {
        state_started,
        state_login,
        state_startsyncing,
        state_offline,
        state_updating_parsers,
        state_ready,
        state_endsync
    } currentState;

public:
    Siilihai(int& argc, char** argv);
    virtual ~Siilihai();
public slots:
    void loginWizardFinished();
    void launchSiilihai();
    void haltSiilihai();
    void cancelProgress();
    void forumAdded(ForumParser fp, ForumSubscription *fs);
    void loginFinished(bool success, QString motd, bool sync);
    void subscribeForum();
    void showSubscribeGroup(ForumSubscription* forum);
    void showUnsubscribeForum(ForumSubscription* forum);
    void subscribeGroupDialogFinished();
    void forumUpdated(ForumSubscription* forumid);
    void updateClicked();
    void updateClicked(ForumSubscription* forumid, bool force=false);
    void cancelClicked();
    void reportClicked(ForumSubscription* forumid);
    void statusChanged(ForumSubscription* forumid, bool reloading, float progress);
    void errorDialog(QString message);
    void listSubscriptionsFinished(QList<int> subscriptions);
    void updateForumParser(ForumParser parser);
    void launchParserMaker();
    void parserMakerClosed();
    void sendParserReportFinished(bool success);
    void offlineModeSet(bool newOffline);
    void syncFinished(bool success);
    void subscriptionFound(ForumSubscription* sub);
    void subscriptionDeleted(ForumSubscription* sub);
    void subscribeForumFinished(ForumSubscription *sub, bool success);
    void userSettingsReceived(bool success, UserSettings *newSettings);
    void settingsChanged(bool byUser);
    void getAuthentication(ForumSubscription *fsub, QAuthenticator *authenticator);
    void updateFailure(ForumSubscription* sub, QString msg);
    void moreMessagesRequested(ForumThread* thread);
private:
    void changeState(siilihai_states newState);
    void launchMainWindow();
    void tryLogin();
    bool endSyncDone;
    bool firstRun;
    LoginWizard *loginWizard;
    SubscribeWizard *subscribeWizard;
    MainWindow *mainWin;
    SiilihaiProtocol protocol;
    QHash <ForumSubscription*, ParserEngine*> engines;
    QSqlDatabase db;
    ForumDatabase fdb;
    ParserDatabase pdb;
    QString baseUrl;
    QSettings settings;
    QList<ForumSubscription*> parsersToUpdateLeft;
    ParserMaker *parserMaker;
    QProgressDialog *progressBar;
    GroupSubscriptionDialog *groupSubscriptionDialog;
    SyncMaster syncmaster;
    UserSettings usettings;
};

#endif /* SIILIHAI_H_ */
