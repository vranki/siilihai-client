#include "loginwizard.h"

LoginWizard::LoginWizard(QWidget *parent, SiilihaiProtocol &proto, QSettings &s) :
	QWizard(parent), protocol(proto), settings(s) {
    setWizardStyle(QWizard::ModernStyle);
#ifndef Q_WS_HILDON
    setPixmap(QWizard::WatermarkPixmap, QPixmap(
            ":/data/siilis_wizard_watermark.png"));
#endif
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    setPage(1, createIntroPage());
    setPage(2, createRegistrationPage());
    setPage(3, createLoginPage());
    setPage(4, createVerifyPage());
    setStartId(1);
    setWindowTitle("Account setup");
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
    registerMessage.setWordWrap(true);
    QFormLayout *layout = new QFormLayout();
    layout->addRow("Username:", &registerUser);
    layout->addRow("E-mail address:", &registerEmail);
    layout->addRow("Password:", &registerPass);
    layout->addRow("Re-enter password:", &registerPass2);
    layout->addRow("Sync status to server:", &enableSync);
    layout->addRow("", &registerMessage);
    page->setLayout(layout);
    connect(&registerUser, SIGNAL(textChanged(QString)), this, SLOT(
            checkRegisterData()));
    connect(&registerEmail, SIGNAL(textChanged(QString)), this, SLOT(
            checkRegisterData()));
    connect(&registerPass, SIGNAL(textChanged(QString)), this, SLOT(
            checkRegisterData()));
    connect(&registerPass2, SIGNAL(textChanged(QString)), this, SLOT(
            checkRegisterData()));
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
    QRadioButton *accountExists = new QRadioButton(
            "I already have a Siilihai account");
    accountDoesntExist.setChecked(true);
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

void LoginWizard::loginFinished(bool success, QString motd, bool sync) {
    qDebug() << Q_FUNC_INFO;
    disconnect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this, SLOT(loginFinished(bool,QString,bool)));
    if(progress) {
	progress->setValue(3);
	progress->deleteLater();
	progress = 0;
    }
    if (!success) {
        if (motd.length() == 0) {
            loginMessage.setText(
                    "Login failed. Check your username, password\nand network connection.");
        } else {
            loginMessage.setText(motd);
        }
        back();
    } else {
        finalLabel.setText(
                "Login successful.\n\nYou are now ready to use Siilihai.");
        settings.setValue("account/username", loginUser.text().trimmed());
        settings.setValue("account/password", loginPass.text().trimmed());
        settings.setValue("preferences/sync_enabled", sync);
        settings.sync();
    }
}

void LoginWizard::registerFinished(bool success, QString motd, bool sync) {
    qDebug() << Q_FUNC_INFO;
    if (progress) {
        progress->setValue(3);
        progress->deleteLater();
        progress = 0;
    }
    if (!success) {
        if (motd.length() == 0) {
            registerMessage.setText(
                    "Unable to register. Check your username, password\nand network connection.");
        } else {
            registerMessage.setText(motd);
        }
        back();
    } else {
        finalLabel.setText(
                "Registration successful.\n\nYou are now ready to use Siilihai.");
        settings.setValue("account/username", registerUser.text().trimmed());
        settings.setValue("account/password", registerPass.text().trimmed());
        settings.setValue("preferences/sync_enabled", sync);
        settings.sync();
    }
}

void LoginWizard::pageChanged(int id) {
    qDebug() << Q_FUNC_INFO << id;
    switch (id) {
    case 2:
        //		checkRegisterData();
        break;
    case 4:
        progress = new QProgressDialog("Connecting siilihai.com..", "Cancel",
                                       0, 3, this);
        progress->setWindowModality(Qt::WindowModal);
        progress->setValue(0);

        disconnect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this, SLOT(loginFinished(bool,QString,bool)));
        disconnect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this, SLOT(registerFinished(bool,QString,bool)));
        if (!accountDoesntExist.isChecked()) {
            connect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this, SLOT(loginFinished(bool, QString,bool)));
            protocol.login(loginUser.text().trimmed(), loginPass.text().trimmed());
        } else {
            connect(&protocol, SIGNAL(loginFinished(bool, QString,bool)), this, SLOT(registerFinished(bool, QString,bool)));
            protocol.registerUser(registerUser.text().trimmed(), registerPass.text().trimmed(),
                                  registerEmail.text().trimmed(), enableSync.isChecked());
        }
        break;
    }
}

int LoginWizard::nextId() const {
    qDebug() << Q_FUNC_INFO;
    switch (currentId()) {
    case 1:
        if (accountDoesntExist.isChecked()) {
            return 2;
        } else {
            return 3;
        }
	case 2:
        return 4;
	case 3:
        return 4;
	default:
        return -1;
    }
}

void LoginWizard::checkRegisterData() {
    qDebug() << Q_FUNC_INFO;
    button(QWizard::NextButton)->setEnabled(false);
    if (registerUser.text().length() < 4) {
        registerMessage.setText(
                "Please type a username. Username can contain only alphanumeric characters.");
        return;
    }

    if (registerEmail.text().length() < 4
        || !registerEmail.text().contains('@') || !registerEmail.text().contains('.')) {
        registerMessage.setText("Please type your e-mail address. It's never displayed for other users.");
        return;
    }
    if (registerPass.text().length() < 4 || registerPass.text()
        != registerPass2.text()) {
        registerMessage.setText(
                "Please type a password twice. Make sure they are same.");
        return;
    }
    registerMessage.setText(
            "Press Next when finished.");
    button(QWizard::NextButton)->setEnabled(true);
}
