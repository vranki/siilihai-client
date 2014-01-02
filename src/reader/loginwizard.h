#ifndef LOGINWIZARD_H
#define LOGINWIZARD_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QProgressDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/siilihaisettings.h>

class LoginWizard: public QWizard {
    Q_OBJECT

public:
    LoginWizard(QWidget *parent, SiilihaiProtocol &proto, SiilihaiSettings &s);
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
    SiilihaiSettings &settings;
    QCheckBox enableSync;
};

#endif // LOGINWIZARD_H
