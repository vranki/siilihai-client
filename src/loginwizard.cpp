#include "loginwizard.h"

LoginWizard::LoginWizard(QWidget *parent, SiilihaiProtocol &proto) :
	QWizard(parent), protocol(proto) {
	setWizardStyle(QWizard::ModernStyle);
	setPixmap(QWizard::WatermarkPixmap, QPixmap(
			"data/siilis_wizard_watermark.png"));
	connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
	setPage(1, createIntroPage());
	setPage(2, createRegistrationPage());
	setPage(3, createLoginPage());
	setPage(4, createVerifyPage());
	setStartId(1);
	setWindowTitle("Account setup");
	connect(&protocol, SIGNAL(loginFinished(bool)), this, SLOT(loginFinished(bool)));
	progress = 0;
	show();
}

LoginWizard::~LoginWizard() {

}

QWizardPage *LoginWizard::createRegistrationPage() {
	QWizardPage *page = new QWizardPage;
	page->setTitle("Registration");

	page->setSubTitle("Please enter your account credentials");
	registerPass.setEchoMode(QLineEdit::Password);
	registerPass2.setEchoMode(QLineEdit::Password);

	QFormLayout *layout = new QFormLayout();
	layout->addRow("Username:", &registerUser);
	layout->addRow("E-mail address:", &registerEmail);
	layout->addRow("Password:", &registerPass);
	layout->addRow("Re-enter password:", &registerPass2);
	layout->addRow("", &registerMessage);
	page->setLayout(layout);
	return page;
}
QWizardPage *LoginWizard::createLoginPage() {
	QWizardPage *page = new QWizardPage;
	page->setTitle("Log in");

	page->setSubTitle("Please enter your account credentials");
	loginPass.setEchoMode(QLineEdit::Password);

	loginUser.setText(settings.value("account/username", "").toString());
	loginPass.setText(settings.value("account/password", "").toString());

	QFormLayout *layout = new QFormLayout();
	layout->addRow("Username:", &loginUser);
	layout->addRow("Password:", &loginPass);
	layout->addRow("", &loginMessage);
	page->setLayout(layout);
	return page;
}

QWizardPage *LoginWizard::createIntroPage() {
	QWizardPage *page = new QWizardPage;
	page->setTitle("Welcome to Siilihai");

	page->setSubTitle("To use Siilihai, you need a account for siilihai.com.");

	accountDoesntExist.setText("Create a new Siilihai account");
	accountDoesntExist.setEnabled(false);
	QRadioButton *accountExists = new QRadioButton(
			"I already have a Siilihai account");
	accountExists->setChecked(true);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(&accountDoesntExist);
	layout->addWidget(accountExists);
	page->setLayout(layout);

	return page;
}

QWizardPage *LoginWizard::createVerifyPage() {
	QWizardPage *page = new QWizardPage;
	page->setTitle("Verifying credentials");

	page->setSubTitle("Your account credentials are being checked");
	finalLabel.setText("Connecting to siilihai.com..");
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(&finalLabel);
	page->setLayout(layout);
	return page;
}

void LoginWizard::loginFinished(bool success) {
	progress->setValue(3);
	progress->deleteLater();
	progress = 0;
	if (!success) {
		loginMessage.setText(
				"Login failed. Check your username, password\nand network connection.");
		back();
	} else {
		finalLabel.setText("Login successful.\n\nYou are now ready to use Siilihai.");
		settings.setValue("account/username", loginUser.text());
		settings.setValue("account/password", loginPass.text());
	}
}

void LoginWizard::pageChanged(int id) {
	if (id == 4) {
		progress = new QProgressDialog("Connecting siilihai.com..", "Cancel",
				0, 3, this);
		progress->setWindowModality(Qt::WindowModal);
		progress->setValue(0);
		if (!accountDoesntExist.isChecked()) {
			protocol.login(loginUser.text(), loginPass.text());
		} else {
			//connect(&protocol, SIGNAL(loginFinished(bool)), this, SLOT(loginFinished(bool)));
			//protocol.login(loginUser.text(), loginPass.text());
		}
	}
}

int LoginWizard::nextId() const {
	switch (currentId()) {
	case 1:
		if (accountDoesntExist.isChecked()) {
			return 2;
		} else {
			return 3;
		}
	case 2:
	case 3:
		return 4;
	default:
		return -1;
	}
}
