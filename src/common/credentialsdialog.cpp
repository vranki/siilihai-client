#include "credentialsdialog.h"
#include <siilihai/credentialsrequest.h>
#include <siilihai/forumdata/forumsubscription.h>

CredentialsDialog::CredentialsDialog(QWidget *parent, CredentialsRequest *cr)
    : QDialog(parent), credentialsRequest(cr)
{
    ui.setupUi(this);
    connect(this, SIGNAL(finished(int)), this, SLOT(dialogClosed()));
    // @todo request type in dialog
    ui.label->setText(QString("Forum %1 requires authentication").arg(credentialsRequest->subscription->alias()));
    connect(credentialsRequest->subscription, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

CredentialsDialog::~CredentialsDialog() {
    if(result()==QDialog::Rejected) { // Make sure username is nulled if dialog is canceled
        credentialsRequest->authenticator.setUser(QString::null);
    }
    credentialsRequest->signalCredentialsEntered(ui.remember->isChecked());
}

void CredentialsDialog::dialogClosed()  {
    credentialsRequest->authenticator.setUser(ui.username->text());
    credentialsRequest->authenticator.setPassword(ui.password->text());
    deleteLater();
}
