#ifndef OPENREQUESTDIALOG_H
#define OPENREQUESTDIALOG_H

#include <QtGui/QDialog>

#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumparser.h>

#include "ui_openrequestdialog.h"

class OpenRequestDialog : public QDialog
{
    Q_OBJECT

public:
    OpenRequestDialog(QWidget *parent, SiilihaiProtocol &p);
    ~OpenRequestDialog();

public slots:
    void listRequestsFinished(QList <ForumRequest> requests);
	void okClicked();
	void cancelClicked();
signals:
	void requestSelected(ForumRequest request);

private:
    SiilihaiProtocol &protocol;
    QHash <int, ForumRequest> tableItems;
    Ui::OpenRequestDialogClass ui;
};

#endif // OPENREQUESTDIALOG_H
