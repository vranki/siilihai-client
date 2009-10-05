#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this,
			SLOT(subscribeForumSlot()));
	connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this,
			SLOT(groupSubscriptionsSlot()));
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
}

void MainWindow::updateForumList() {
	while (ui.forumToolBox->count() > 0) {
		ui.forumToolBox->removeItem(0);
	}
	forumItems.clear();
	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		QListWidget *lw = new QListWidget(this);
		QList<ForumGroup> groups = fdb.listGroups(forums[i].parser);
		for (int j = 0; j < groups.size(); j++) {
			if (groups[j].subscribed) {
				QListWidgetItem *lwi = new QListWidgetItem(lw);
				lwi->setText(groups[j].name);
				lwi->setIcon(QIcon("data/folder.svg"));
				lw->addItem(lwi);
				forumGroups[lwi] = groups[j];
			}
		}
		connect(
				lw,
				SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
				this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
		ui.forumToolBox->addItem(lw, forums[i].name);
		forumItems[forums[i].parser] = i;
		setForumStatus(forums[i].parser, false);
	}
}

MainWindow::~MainWindow() {

}

void MainWindow::subscribeForumSlot() {
	emit subscribeForum();
}

void MainWindow::updateClickedSlot() {
	emit updateClicked();
}

void MainWindow::cancelClickedSlot() {
	emit cancelClicked();
}

void MainWindow::groupSubscriptionsSlot() {
	int f = getSelectedForum();
	if (f > 0)
		emit groupSubscriptions(f);
}

int MainWindow::getSelectedForum() {
	int s = ui.forumToolBox->currentIndex();
	QHashIterator<int, int> hi(forumItems);
	while (hi.hasNext()) {
		hi.next();
		if (hi.value() == s)
			return hi.key();
	}
	return -1;
}

void MainWindow::setForumStatus(int forum, bool reloading) {
	if (!forumItems.contains(forum))
		return;

	if (reloading) {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon(
				"data/stock_refresh.svg"));
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(false);
	} else {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon(
				"data/emblem-web.svg"));
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(true);
	}
}

void MainWindow::groupSelected(QListWidgetItem* item, QListWidgetItem *prev) {
	ui.threadTree->clear();
	QString group = forumGroups[item].id;
	int forum = forumGroups[item].parser;
	qDebug() << "Selected group " << group << " in forum " << forum;
	QList<ForumThread> threads = fdb.listThreads(forumGroups[item]);
	QList<QTreeWidgetItem *> items;
	for (int i = 0; i < threads.size(); ++i) {
		QStringList header;
		ForumThread *thread = &threads[i];

		QList<ForumMessage> messages = fdb.listMessages(threads[i]);
		if (messages.size() > 0) {
			header << messages[0].subject << messages[0].lastchange
					<< messages[0].author;
		} else {
			header << thread->name << thread->lastchange;
		}
		QTreeWidgetItem *threadItem =
				new QTreeWidgetItem(ui.threadTree, header);

		for (int m = 1; m < messages.size(); ++m) {
			ForumMessage *message = &messages[m];
			QStringList messageHeader;
			messageHeader << message->subject << message->lastchange
					<< message->author;
			QTreeWidgetItem *messageItem = new QTreeWidgetItem(threadItem,
					messageHeader);
			threadItem->addChild(messageItem);
		}
		items.append(threadItem);
	}
	ui.threadTree->insertTopLevelItems(0, items);
	ui.threadTree->resizeColumnToContents(0);
	ui.threadTree->resizeColumnToContents(1);
	ui.threadTree->resizeColumnToContents(2);
}
