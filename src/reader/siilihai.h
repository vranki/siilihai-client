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
class CredentialsRequest;

class Siilihai: public ClientLogic {
    Q_OBJECT

public:
    Siilihai();
    virtual ~Siilihai();
private slots:
    virtual void subscribeForum();
    virtual void loginWizardFinished();
    virtual void settingsChanged(bool);
    void cancelProgress();
    void showUnsubscribeForum(ForumSubscription* forum);
    void reportClicked(ForumSubscription* forumid);
    void launchParserMaker();
    void parserMakerClosed();
    void sendParserReportFinished(bool success);
    void subscribeGroupDialogFinished();
    virtual void showStatusMessage(QString message);
protected:
    virtual QString getDataFilePath();
    virtual void errorDialog(QString message);
    virtual void showLoginWizard();
    virtual void showSubscribeGroup(ForumSubscription* forum);
    virtual void showCredentialsDialog(CredentialsRequest *cr);
    virtual void changeState(siilihai_states newState);
    virtual void closeUi();
    virtual void showMainWindow();
private:
    LoginWizard *loginWizard;
    MainWindow *mainWin;
    ParserMaker *parserMaker;
    GroupSubscriptionDialog *groupSubscriptionDialog;
};

#endif /* SIILIHAI_H_ */
