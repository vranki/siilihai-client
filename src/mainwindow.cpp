#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this,
			SLOT(subscribeForumSlot()));
	connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this,
			SLOT(groupSubscriptionsSlot()));
	connect(ui.actionUpdate_all, SIGNAL(triggered()), this,
			SLOT(updateClickedSlot()));
	connect(ui.actionUpdate_selected, SIGNAL(triggered()), this,
			SLOT(updateSelectedClickedSlot()));
	connect(ui.actionUnsubscribe, SIGNAL(triggered()), this,
			SLOT(unsubscribeForumSlot()));
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
	busyForums = 0;
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
				lwi->setIcon(QIcon(":/data/folder.png"));
				lw->addItem(lwi);
				forumGroups[lwi] = groups[j];
			}
		}
		disconnect(
				lw,
				SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
				this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
		connect(
				lw,
				SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
				this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
		ui.forumToolBox->addItem(lw, forums[i].name);
		forumItems[forums[i].parser] = i;
		ui.forumToolBox->setItemIcon(i, QIcon(":/data/emblem-web.png"));
		ui.forumToolBox->widget(i)->setEnabled(true);
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

void MainWindow::updateSelectedClickedSlot() {
	if (getSelectedForum() > 0)
		emit updateClicked(getSelectedForum());
}

void MainWindow::unsubscribeForumSlot() {
	if (getSelectedForum() > 0)
		emit unsubscribeForum(getSelectedForum());
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
	if (reloading) {
		busyForums++;
	} else {
		busyForums--;
	}
	qDebug() << "Busy forums now " << busyForums;
	ui.stopButton->setEnabled(busyForums > 0);
	ui.updateButton->setEnabled(busyForums == 0);

	if (!forumItems.contains(forum))
		return;

	if (reloading) {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon(
				":/data/view-refresh.png"));
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(false);
	} else {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon(
				":/data/emblem-web.png"));
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(true);
	}
}

void MainWindow::groupSelected(QListWidgetItem* item, QListWidgetItem *prev) {
	ui.threadTree->clear();
	forumMessages.clear();
	QString group = forumGroups[item].id;
	// qDebug() << "Selected group " << group << " in forum " << forum;
	QList<ForumThread> threads = fdb.listThreads(forumGroups[item]);
	QList<QTreeWidgetItem *> items;
	for (int i = 0; i < threads.size(); ++i) {
		QStringList header;
		ForumThread *thread = &threads[i];

		QList<ForumMessage> messages = fdb.listMessages(threads[i]);
		ForumMessage threadHeaderMessage;
		if (messages.size() > 0) { // Thread contains messages
			threadHeaderMessage = messages[0];

			// Sometimes messages don't have a subject - only threads do.
			// Check for this:
			if (threadHeaderMessage.subject.length() == 0)
				threadHeaderMessage.subject = thread->name;

		} else { // Thread doesn't contain messages (wat?)
			threadHeaderMessage.subject = thread->name;
			threadHeaderMessage.lastchange = thread->lastchange;
		}
		header << threadHeaderMessage.subject << threadHeaderMessage.lastchange
				<< threadHeaderMessage.author;

		QTreeWidgetItem *threadItem =
				new QTreeWidgetItem(ui.threadTree, header);
		if (messages.size() > 0) {
			forumMessages[threadItem] = messages[0];

			for (int m = 1; m < messages.size(); ++m) {
				ForumMessage *message = &messages[m];
				QStringList messageHeader;
				if (message->subject.length() == 0) {
					messageHeader << "Re: " + threadHeaderMessage.subject;
				} else {
					messageHeader << message->subject;
				}
				messageHeader << message->lastchange << message->author;
				QTreeWidgetItem *messageItem = new QTreeWidgetItem(threadItem,
						messageHeader);

				threadItem->addChild(messageItem);
				forumMessages[messageItem] = messages[m];
				updateMessageRead(messageItem);
			}
			items.append(threadItem);
			updateMessageRead(threadItem);
		}
	}
	ui.threadTree->insertTopLevelItems(0, items);
	ui.threadTree->resizeColumnToContents(0);
	ui.threadTree->resizeColumnToContents(1);
	ui.threadTree->resizeColumnToContents(2);
	disconnect(ui.threadTree,
			SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
			this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
	connect(ui.threadTree,
			SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
			this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
}

void MainWindow::messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev) {
	if (item == 0)
		return;

	ForumMessage * msg = &forumMessages[item];
	QString
			html =
					"<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\"></head><body>"
							+ msg->body.toLatin1() + "</body>";
	ui.webView->setContent(html.toUtf8(), QString("text/html"), QUrl("/"));
	ui.messageAuthor->setText(msg->author);
	ui.messageSubject->setText(msg->subject);
	ui.messageDate->setText(msg->lastchange);
	msg->read = true;
	emit
	messageRead(forumMessages[item]);
	updateMessageRead(item);
}

void MainWindow::updateMessageRead(QTreeWidgetItem *item) {
	QFont font = item->font(0);
	if (forumMessages[item].read) {
		font.setBold(false);
	} else {
		font.setBold(true);
	}
	item->setFont(0, font);
}
