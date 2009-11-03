#include "groupsubscriptiondialog.h"

GroupSubscriptionDialog::GroupSubscriptionDialog(QWidget *parent) :
	QDialog(parent) {
	ui.setupUi(this);
	fdb = 0;
	connect(ui.selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
	connect(ui.selectNoneButton, SIGNAL(clicked()), this, SLOT(selectNone()));
	connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(apply()));
}

GroupSubscriptionDialog::~GroupSubscriptionDialog() {

}
void GroupSubscriptionDialog::selectAll() {
	QHashIterator<QString, QListWidgetItem*> i(listItems);
	 while (i.hasNext()) {
	     i.next();
	     i.value()->setCheckState(Qt::Checked);
	 }
}

void GroupSubscriptionDialog::selectNone() {
	QHashIterator<QString, QListWidgetItem*> i(listItems);
	 while (i.hasNext()) {
	     i.next();
	     i.value()->setCheckState(Qt::Unchecked);
	 }
}

void GroupSubscriptionDialog::apply() {
	QList<ForumGroup> subscriptions = fdb->listGroups(forum);
	for (int i = 0; i < subscriptions.size(); i++) {
		QListWidgetItem *item = listItems[subscriptions[i].id];
		bool itemChecked = (item->checkState()==Qt::Checked);
		if(itemChecked != subscriptions[i].subscribed) {
			subscriptions[i].subscribed = itemChecked;
			fdb->updateGroup(subscriptions[i]);
		}
	}
	close();
}

void GroupSubscriptionDialog::updateList() {
	ui.listWidget->clear();
	listItems.clear();
	QList<ForumGroup> subscriptions = fdb->listGroups(forum);
	for (int i = 0; i < subscriptions.size(); i++) {
		QListWidgetItem *newItem = new QListWidgetItem();
		newItem->setText(subscriptions[i].name);
		newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		if (subscriptions[i].subscribed) {
			newItem->setCheckState(Qt::Checked);
		} else {
			newItem->setCheckState(Qt::Unchecked);
		}
		ui.listWidget->addItem(newItem);
		listItems[subscriptions[i].id] = newItem;
	}
}

void GroupSubscriptionDialog::setForum(ForumDatabase *db, int forumId) {
	fdb = db;
	forum = forumId;
	updateList();
}
