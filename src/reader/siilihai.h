#ifndef SIILIHAI_H_
#define SIILIHAI_H_
#include <QObject>

#include <QMessageBox>
#include <QProgressDialog>
#include <siilihai/clientlogic.h>

#ifndef Q_WS_HILDON
#include "../parsermaker/parsermaker.h"
#else
class ParserMaker;
#endif

class LoginWizard;
class MainWindow;
class SyncMaster;
class GroupSubscriptionDialog;

class Siilihai: public ClientLogic {
    Q_OBJECT

public:
    Siilihai();
    virtual ~Siilihai();
private slots:
    void launchSiilihai();
    void cancelProgress();
    virtual void loginFinished(bool success, QString motd, bool sync);
    virtual void subscribeForum();
    void showSubscribeGroup(ForumSubscription* forum);
    void showUnsubscribeForum(ForumSubscription* forum);
    void reportClicked(ForumSubscription* forumid);
    void statusChanged(ForumSubscription* forumid, bool reloading, float progress);
    void launchParserMaker();
    void parserMakerClosed();
    void sendParserReportFinished(bool success);
    void getAuthentication(ForumSubscription *fsub, QAuthenticator *authenticator);
    void subscribeGroupDialogFinished();
    virtual void parserEngineStateChanged(ParserEngine *engine, ParserEngine::ParserEngineState newState, ParserEngine::ParserEngineState oldState);
protected:
    virtual QString getDataFilePath();
    virtual void errorDialog(QString message);
    virtual void showLoginWizard();
    virtual void changeState(siilihai_states newState);
    virtual void closeUi();
    virtual void showMainWindow();
    virtual void subscriptionFound(ForumSubscription* sub);
private:
    LoginWizard *loginWizard;
    MainWindow *mainWin;
    ParserMaker *parserMaker;
    QProgressDialog *progressBar;
    GroupSubscriptionDialog *groupSubscriptionDialog;
};

#endif /* SIILIHAI_H_ */
