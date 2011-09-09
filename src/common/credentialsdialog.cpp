#include "credentialsdialog.h"

CredentialsDialog::CredentialsDialog(QWidget *parent, ForumSubscription *sub, QAuthenticator *authenticator, QSettings *set)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(acceptClicked()));
    ui.label->setText(QString("Forum %1 requires authentication").arg(sub->alias()));
    auth = authenticator;
    settings = set;
    subscription = sub;
    connect(subscription, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

CredentialsDialog::~CredentialsDialog() {
}

void CredentialsDialog::acceptClicked()  {
    auth->setUser(ui.username->text());
    auth->setPassword(ui.password->text());
    if(ui.remember->isChecked()) {
        if(settings) {
            settings->beginGroup("authentication");
            settings->beginGroup(QString::number(subscription->parser()));
            settings->setValue("username", ui.username->text());
            settings->setValue("password", ui.password->text());
            settings->setValue("failed", "false");
            settings->endGroup();
            settings->endGroup();
            settings->sync();
        }
    }
    emit credentialsEntered(auth);
}
