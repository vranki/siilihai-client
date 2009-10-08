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

		// Setup Favicon
		if (forumIcons.contains(forums[i].parser)) {
			forumIcons[forums[i].parser]->update();
		} else {
			QString fiUrl = pdb.getParser(forums[i].parser).forum_url;
			fiUrl = fiUrl.replace(QUrl(fiUrl).path(), "");
			fiUrl = fiUrl + "/favicon.ico";
			Favicon *fi = new Favicon(this, forums[i].parser);
			connect(fi, SIGNAL(iconChanged(int, QIcon)), this,
					SLOT(iconUpdated(int, QIcon)));
			fi->fetchIcon(QUrl(QUrl(fiUrl)), QPixmap(":/data/emblem-web.png"));
			forumIcons[forums[i].parser] = fi;
		}
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
	Q_ASSERT(0 < busyForums < forumItems.size());
	ui.stopButton->setEnabled(busyForums > 0);
	ui.updateButton->setEnabled(busyForums == 0);

	if (!forumItems.contains(forum))
		return;
	if (!forumIcons.contains(forum))
		return;
	Favicon *fi = forumIcons[forum];

	if (reloading) {
		fi->setReloading(true);
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(false);
		ui.forumToolBox->setItemEnabled(forumItems[forum], false);
	} else {
		fi->setReloading(false);
		ui.forumToolBox->widget(forumItems[forum])->setEnabled(true);
		ui.forumToolBox->setItemEnabled(forumItems[forum], true);
	}
	if (busyForums > 0) {
		ui.statusbar->showMessage("Updating Forums, please wait.. "
				+ QString().number(busyForums), 5000);
	} else {
		ui.statusbar->showMessage("Forums updated", 5000);
	}
}

void MainWindow::groupSelected(QListWidgetItem* item, QListWidgetItem *prev) {
	ui.threadTree->clear();
	forumMessages.clear();
	QString group = forumGroups[item].id;
	QList<ForumThread> threads = fdb.listThreads(forumGroups[item]);
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

void MainWindow::iconUpdated(int forum, QIcon newIcon) {
	if (forumItems.contains(forum))
		ui.forumToolBox->setItemIcon(forumItems[forum], newIcon);
}
