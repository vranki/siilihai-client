#include "groupsubscriptiondialog.h"
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/forumdatabase/forumdatabase.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/messageformatting.h>

GroupSubscriptionDialog::GroupSubscriptionDialog(QWidget *parent) : QDialog(parent) {
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
    for(ForumGroup *group : forum->values()) {
        QListWidgetItem *item = listItems[group];
        if(item) {
            bool itemChecked = (item->checkState()==Qt::Checked);
            if(itemChecked != group->isSubscribed()) {
                group->setSubscribed(itemChecked);
                group->setChangeset(0);
                group->setHasChanged(true);
                group->commitChanges();
            }
        } else {
            // Groups have changed while dialog has been open..
        }
    }
    emit updateGroupSubscriptions(forum);
    close();
    deleteLater();
}

void GroupSubscriptionDialog::setForum(ForumDatabase *db, ForumSubscription *nforum) {
    fdb = db;
    if(forum) forum->disconnect(this);
    forum = nforum;
    connect(fdb, SIGNAL(subscriptionRemoved(ForumSubscription*)), this, SLOT(subscriptionDeleted(ForumSubscription*)));
    ui.listWidget->clear();
    listItems.clear();
    if(!forum || (forum->updateEngine()->state() == ParserEngine::UES_UPDATING)) {
        close();
        return;
    }
    for(ForumGroup *group : forum->values()) {
        QListWidgetItem *newItem = new QListWidgetItem();

        QString itemLabel = group->name();
        if(!group->hierarchy().isEmpty())
            itemLabel += " [" + group->hierarchy() + "]";
        newItem->setText(MessageFormatting::stripHtml(itemLabel));
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        newItem->setCheckState(group->isSubscribed() ? Qt::Checked : Qt::Unchecked);
        ui.listWidget->addItem(newItem);
        listItems[group] = newItem;
    }
}

ForumSubscription* GroupSubscriptionDialog::subscription() {
    return forum;
}

void GroupSubscriptionDialog::subscriptionDeleted(ForumSubscription *sub) {
    if(sub==forum) {
        forum=0;
        close();
        deleteLater();
    }
}
