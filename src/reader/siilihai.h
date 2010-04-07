#ifndef SIILIHAI_H_
#define SIILIHAI_H_
#include <QObject>
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

#include "loginwizard.h"
#include "subscribewizard.h"
#include "mainwindow.h"
#include "groupsubscriptiondialog.h"
#include "forumlistwidget.h"
#include "reportparser.h"

#ifndef Q_WS_HILDON
#include "../parsermaker/parsermaker.h"
#else
class ParserMaker;
#endif

#define DATABASE_FILE "/.siilihai.db"
#define BASEURL "http://www.siilihai.com/"

// Login process:
// Login -> update parsers -> sync -> ready
//       '> offline        '> ready

class Siilihai: public QObject {
    Q_OBJECT

public:
    Siilihai();
    virtual ~Siilihai();
public slots:
    void loginWizardFinished();
    void launchSiilihai();
    void haltSiilihai();
    void forumAdded(ForumParser fp, ForumSubscription *fs);
    void loginFinished(bool success, QString motd=QString());
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
private:
    void launchMainWindow();
    void updateState();
    void tryLogin();

    LoginWizard *loginWizard;
    SubscribeWizard *subscribeWizard;
    MainWindow *mainWin;
    SiilihaiProtocol protocol;
    QHash <ForumSubscription*, ParserEngine*> engines;
    QSqlDatabase db;
    ForumDatabase fdb;
    ParserDatabase pdb;
    QString baseUrl;
    bool readerReady, offlineMode, syncEnabled, quitting;
    QSettings settings;
    QList<ForumSubscription*> parsersToUpdateLeft;
    ParserMaker *parserMaker;
    QProgressDialog *loginProgress;
    GroupSubscriptionDialog *groupSubscriptionDialog;
    SyncMaster syncmaster;
};

#endif /* SIILIHAI_H_ */
