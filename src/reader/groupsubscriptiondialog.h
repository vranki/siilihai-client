#ifndef GROUPSUBSCRIPTIONDIALOG_H
#define GROUPSUBSCRIPTIONDIALOG_H

#include <QtGui/QDialog>
#include <QHash>
#include <siilihai/forumdatabase.h>
#include <siilihai/forumsubscription.h>

#include "ui_groupsubscriptiondialog.h"

class GroupSubscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    GroupSubscriptionDialog(QWidget *parent = 0);
    ~GroupSubscriptionDialog();
    void setForum(ForumDatabase *db, int forumId);
    void updateList();
public slots:
	void selectAll();
	void selectNone();
	void apply();
private:
    Ui::GroupSubscriptionDialogClass ui;
    ForumDatabase *fdb;
    int forum;
    QHash<QString, QListWidgetItem*> listItems;
};

#endif // GROUPSUBSCRIPTIONDIALOG_H
