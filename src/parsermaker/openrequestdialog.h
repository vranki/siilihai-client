#ifndef OPENREQUESTDIALOG_H
#define OPENREQUESTDIALOG_H

#include <QDialog>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/parser/forumparser.h>

#include "ui_openrequestdialog.h"

class OpenRequestDialog : public QDialog
{
    Q_OBJECT

public:
    OpenRequestDialog(QWidget *parent, SiilihaiProtocol &p);
    ~OpenRequestDialog();

private slots:
    void listRequestsFinished(QList <ForumRequest*> requests);
    void okClicked();
    void cancelClicked();
signals:
    void requestSelected(ForumRequest *request);

private:
    SiilihaiProtocol &protocol;
    QHash <int, ForumRequest*> tableItems;
    Ui::OpenRequestDialogClass ui;
};

#endif // OPENREQUESTDIALOG_H
