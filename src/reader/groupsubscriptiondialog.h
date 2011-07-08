#ifndef GROUPSUBSCRIPTIONDIALOG_H
#define GROUPSUBSCRIPTIONDIALOG_H

#include <QtGui/QDialog>
#include <QHash>
class ForumDatabase;
class ForumSubscription;
class ForumGroup;

#include "ui_groupsubscriptiondialog.h"

class GroupSubscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    GroupSubscriptionDialog(QWidget *parent = 0);
    ~GroupSubscriptionDialog();
    void setForum(ForumDatabase *db, ForumSubscription *fs);
    ForumSubscription* subscription(); // May be null
public slots:
    void selectAll();
    void selectNone();
    void apply();
    void subscriptionDeleted(ForumSubscription *sub);
private:
    Ui::GroupSubscriptionDialogClass ui;
    ForumDatabase *fdb;
    ForumSubscription *forum;
    QHash<ForumGroup*, QListWidgetItem*> listItems;
};

#endif // GROUPSUBSCRIPTIONDIALOG_H
