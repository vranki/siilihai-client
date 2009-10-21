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
	connect(ui.viewInBrowser, SIGNAL(clicked()), this,
			SLOT(viewInBrowserClickedSlot()));
	flw = new ForumListWidget(this, fdb, pdb);
	ui.forumsSplitter->insertWidget(0, flw);
	connect(flw, SIGNAL(groupSelected(ForumGroup)), this, SLOT(groupSelected(ForumGroup)));

	busyForums = 0;
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
	int selectedForum = flw->getSelectedForum();
	if (selectedForum > 0)
		emit updateClicked(selectedForum);
}

void MainWindow::unsubscribeForumSlot() {
	int selectedForum = flw->getSelectedForum();
	if (selectedForum > 0)
		emit unsubscribeForum(selectedForum);
}

void MainWindow::cancelClickedSlot() {
	emit cancelClicked();
}

void MainWindow::groupSubscriptionsSlot() {
	int selectedForum = flw->getSelectedForum();
	if (selectedForum > 0)
		emit groupSubscriptions(selectedForum);
}


void MainWindow::groupSelected(ForumGroup fg) {
	ui.threadTree->clear();
	forumMessages.clear();
	QList<ForumThread> threads = fdb.listThreads(fg);
	QList<QTreeWidgetItem *> items;
	for (int i = 0; i < threads.size(); ++i) {
		QStringList header;
		ForumThread *thread = &threads[i];
		Q_ASSERT(thread->isSane());

		QList<ForumMessage> messages = fdb.listMessages(threads[i]);
		ForumMessage threadHeaderMessage;
		if (messages.size() > 0) { // Thread contains messages
			threadHeaderMessage = messages[0];
			Q_ASSERT(messages[0].isSane());
			Q_ASSERT(threadHeaderMessage.isSane());
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
				Q_ASSERT(message->isSane());
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
	if (!forumMessages.contains(item)) {
		qDebug() << "A thread with no messages. Broken parser?.";
		return;
	}
	displayedMessage = forumMessages[item];
	ForumMessage *msg = &forumMessages[item];
	Q_ASSERT(msg->isSane());
	QString
			html =
					"<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\"></head><body>"
							+ msg->body + "</body>";

	QString baseUrl = msg->url;
	int i = baseUrl.lastIndexOf('/');
	if (i > 0) {
		baseUrl = baseUrl.left(i + 1);
	}
	ui.webView->setContent(html.toUtf8(), QString("text/html"), QUrl(baseUrl));
	ui.messageAuthor->setText(msg->author);
	ui.messageSubject->setText(msg->subject);
	ui.messageDate->setText(msg->lastchange);
	msg->read = true;
	emit
	messageRead(forumMessages[item]);
	updateMessageRead(item);
	flw->updateReadCounts();
}

void MainWindow::updateMessageRead(QTreeWidgetItem *item) {
	QFont font = item->font(0);
	if (forumMessages[item].read) {
		font.setBold(false);
		item->setIcon(0, QIcon(":/data/emblem-mail.png"));
	} else {
		font.setBold(true);
		item->setIcon(0, QIcon(":/data/mail-unread.png"));
	}
	item->setFont(0, font);
}

void MainWindow::viewInBrowserClickedSlot() {
	if (!displayedMessage.isSane())
		return;
	qDebug() << "Launching browser to " << displayedMessage.url;
	QDesktopServices::openUrl(displayedMessage.url);
}

void MainWindow::setForumStatus(int forum, bool reloading) {
	if (reloading) {
		busyForums++;
	} else {
		busyForums--;
	}
	ui.stopButton->setEnabled(busyForums > 0);
	ui.updateButton->setEnabled(busyForums == 0);
	if (busyForums > 0) {
		ui.statusbar->showMessage("Updating Forums, please wait.. "
				+ QString().number(busyForums), 5000);
	} else {
		ui.statusbar->showMessage("Forums updated", 5000);
	}

	flw->setForumStatus(forum, reloading);
}

ForumListWidget* MainWindow::forumList() {
	return flw;
}
