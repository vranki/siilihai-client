#ifndef CREDENTIALSDIALOG_H
#define CREDENTIALSDIALOG_H
// Qt5
#include <QDialog>
#include "ui_credentialsdialog.h"

class CredentialsRequest;

class CredentialsDialog : public QDialog
{
    Q_OBJECT

public:
    CredentialsDialog(QWidget *parent, CredentialsRequest *cr);
    ~CredentialsDialog();
private slots:
    void acceptClicked();

private:
    Ui::CredentialsDialog ui;
    CredentialsRequest *credentialsRequest;
};

#endif
