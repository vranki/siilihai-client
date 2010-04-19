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
    void setForum(ForumDatabase *db, ForumSubscription *fs);
    ForumSubscription* subscription();
public slots:
    void selectAll();
    void selectNone();
    void apply();
private:
    Ui::GroupSubscriptionDialogClass ui;
    ForumDatabase *fdb;
    ForumSubscription *forum;
    QHash<ForumGroup*, QListWidgetItem*> listItems;
};

#endif // GROUPSUBSCRIPTIONDIALOG_H
