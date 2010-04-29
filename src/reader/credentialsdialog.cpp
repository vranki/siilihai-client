#include "credentialsdialog.h"

CredentialsDialog::CredentialsDialog(QWidget *parent, ForumSubscription *sub, QAuthenticator *authenticator)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(acceptClicked()));
    ui.label->setText(QString("Forum %1 requires authentication").arg(sub->alias()));
    auth = authenticator;
}

CredentialsDialog::~CredentialsDialog()
{

}

void CredentialsDialog::acceptClicked()  {
    auth->setUser(ui.username->text());
    auth->setPassword(ui.password->text());
}
