#include "groupsubscriptiondialog.h"
#include "siilihai/forumgroup.h"

GroupSubscriptionDialog::GroupSubscriptionDialog(QWidget *parent) :
	QDialog(parent) {
    ui.setupUi(this);
    fdb = 0;
    forum = 0;
    connect(ui.selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(ui.selectNoneButton, SIGNAL(clicked()), this, SLOT(selectNone()));
    connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(apply()));
}

GroupSubscriptionDialog::~GroupSubscriptionDialog() {

}
void GroupSubscriptionDialog::selectAll() {
    QHashIterator<ForumGroup*, QListWidgetItem*> i(listItems);
    while (i.hasNext()) {
        i.next();
        i.value()->setCheckState(Qt::Checked);
    }
}

void GroupSubscriptionDialog::selectNone() {
    QHashIterator<ForumGroup*, QListWidgetItem*> i(listItems);
    while (i.hasNext()) {
        i.next();
        i.value()->setCheckState(Qt::Unchecked);
    }
}

void GroupSubscriptionDialog::apply() {
    foreach(ForumGroup *group, forum->values()) {
        QListWidgetItem *item = listItems[group];
        bool itemChecked = (item->checkState()==Qt::Checked);
        if(itemChecked != group->isSubscribed()) {
            group->setSubscribed(itemChecked);
            group->setChangeset(0);
            group->setHasChanged(true);
            group->commitChanges();
        }
    }
    close();
}

void GroupSubscriptionDialog::setForum(ForumDatabase *db, ForumSubscription *nforum) {
    fdb = db;
    forum = nforum;

    ui.listWidget->clear();
    listItems.clear();
    foreach(ForumGroup *group, forum->values()) {
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText(group->name());
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        if (group->isSubscribed()) {
            newItem->setCheckState(Qt::Checked);
        } else {
            newItem->setCheckState(Qt::Unchecked);
        }
        ui.listWidget->addItem(newItem);
        listItems[group] = newItem;
    }
}

ForumSubscription* GroupSubscriptionDialog::subscription() {
    return forum;
}
