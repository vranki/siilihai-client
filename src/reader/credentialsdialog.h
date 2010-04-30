#ifndef CREDENTIALSDIALOG_H
#define CREDENTIALSDIALOG_H

#include <QtGui/QDialog>
#include <QAuthenticator>
#include <QSettings>
#include <siilihai/forumsubscription.h>

#include "ui_credentialsdialog.h"

class CredentialsDialog : public QDialog
{
    Q_OBJECT

public:
    CredentialsDialog(QWidget *parent, ForumSubscription *sub, QAuthenticator *authenticator, QSettings *set);
    ~CredentialsDialog();
public slots:
    void acceptClicked();

private:
    Ui::CredentialsDialog ui;
    QAuthenticator *auth;
    QSettings *settings;
    ForumSubscription *subscription;
};

#endif
