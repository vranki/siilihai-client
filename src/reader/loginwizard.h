#ifndef LOGINWIZARD_H
#define LOGINWIZARD_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QProgressDialog>
#include <siilihai/siilihaiprotocol.h>

class LoginWizard: public QWizard {
    Q_OBJECT

public:
    LoginWizard(QWidget *parent, SiilihaiProtocol &proto, QSettings &s);
    ~LoginWizard();
    QWizardPage *createIntroPage();
    QWizardPage *createRegistrationPage();
    QWizardPage *createLoginPage();
    QWizardPage *createVerifyPage();

    int nextId() const;
public slots:
    void pageChanged(int id);
    void loginFinished(bool success, QString motd, bool sync);
    void registerFinished(bool success, QString motd, bool sync);
    void checkRegisterData();

private:
    QWizard wizard;
    QRadioButton accountDoesntExist, accountExists, noAccount;
    QLineEdit loginUser, loginPass, registerUser, registerPass, registerPass2, registerEmail;
    SiilihaiProtocol &protocol;
    QProgressDialog *progress;
    QLabel loginMessage, registerMessage, finalLabel;
    QSettings &settings;
    QCheckBox enableSync;
};

#endif // LOGINWIZARD_H
