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

/**
 * @brief The Siilihai class is the main GUI godclass for the desktop reader
 */
class Siilihai: public ClientLogic {
    Q_OBJECT

public:
    Siilihai();
    virtual ~Siilihai();

private slots:
    virtual void subscribeForum();
    virtual void loginWizardFinished();
    virtual void settingsChanged(bool byUser=true);
    void showUnsubscribeForum(ForumSubscription* forum);
    void reportClicked(ForumSubscription* forumid);
    void launchParserMaker();
    void parserMakerClosed();
    void sendParserReportFinished(bool success);
    virtual void showStatusMessage(QString message);
    void showLoginWizardSlot();
    void showCredentialsDialogSlot();

protected:
    virtual void errorDialog(QString message);
    virtual void changeState(siilihai_states newState);
    virtual void closeUi();
    virtual void showMainWindow();

private slots:
    virtual void showGroupSubscriptionDialog(ForumSubscription* forum);

private:
    LoginWizard *loginWizard;
    MainWindow *mainWin;
    ParserMaker *parserMaker;
};

#endif /* SIILIHAI_H_ */
