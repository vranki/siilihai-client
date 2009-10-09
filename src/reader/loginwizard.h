#ifndef LOGINWIZARD_H
#define LOGINWIZARD_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <siilihaiprotocol.h>

class LoginWizard: public QWizard {
	Q_OBJECT

public:
	LoginWizard(QWidget *parent, SiilihaiProtocol &proto);
	~LoginWizard();
	QWizardPage *createIntroPage();
	QWizardPage *createRegistrationPage();
	QWizardPage *createLoginPage();
	QWizardPage *createVerifyPage();

	int nextId() const;
public slots:
	void pageChanged(int id);
	void loginFinished(bool success);
private:
	QWizard wizard;
	QRadioButton accountDoesntExist;
	QLineEdit loginUser, loginPass, registerUser, registerPass, registerPass2, registerEmail;
	SiilihaiProtocol &protocol;
	QProgressDialog *progress;
	QLabel loginMessage, registerMessage, finalLabel;
	QSettings settings;
};

#endif // LOGINWIZARD_H
