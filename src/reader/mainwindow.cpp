#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QSettings *s, QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	readerReady = false;
	settings = s;
	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this,
			SLOT(subscribeForumSlot()));
	connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this,
			SLOT(groupSubscriptionsSlot()));
	connect(ui.actionUpdate_all, SIGNAL(triggered()), this,
			SLOT(updateClickedSlot()));
	connect(ui.actionUpdate_selected, SIGNAL(triggered()), this,
			SLOT(updateSelectedClickedSlot()));
	connect(ui.actionReport_broken_or_working, SIGNAL(triggered()), this,
			SLOT(reportClickedSlot()));
	connect(ui.actionUnsubscribe, SIGNAL(triggered()), this,
			SLOT(unsubscribeForumSlot()));
	connect(ui.actionParser_Maker, SIGNAL(triggered()), this,
			SLOT(launchParserMakerSlot()));
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
	connect(ui.hideButton, SIGNAL(clicked()), this, SLOT(hideClickedSlot()));
	connect(ui.viewInBrowser, SIGNAL(clicked()), this,
			SLOT(viewInBrowserClickedSlot()));
	flw = new ForumListWidget(this, fdb, pdb);
	ui.forumsSplitter->insertWidget(0, flw);
	flw->setEnabled(false);

	connect(flw, SIGNAL(groupSelected(ForumGroup)), this, SLOT(groupSelected(ForumGroup)));
	if(!restoreGeometry(settings->value("reader_geometry").toByteArray()))
		showMaximized();
#ifdef Q_WS_HILDON
	ui.headerFrame->hide();
	ui.updateButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	ui.stopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	ui.hideButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	ui.threadTree->setHeaderHidden(true);
#else
	ui.hideButton->hide();
#endif
	hideClickedSlot();
	hideClickedSlot();
}

MainWindow::~MainWindow() {
}

void MainWindow::subscribeForumSlot() {
	emit subscribeForum();
}

void MainWindow::launchParserMakerSlot() {
	emit launchParserMaker();
}

void MainWindow::updateClickedSlot() {
	emit updateClicked();
}

void MainWindow::reportClickedSlot() {
	int selectedForum = flw->getSelectedForum();
	emit reportClicked(selectedForum);
}

void MainWindow::hideClickedSlot() {

	if(flw->isHidden()) {
		flw->show();
		ui.hideButton->setText("Hide");
		ui.hideButton->setIcon(QIcon(":/data/go-first.png"));
	} else {
		flw->hide();
		ui.hideButton->setIcon(QIcon(":/data/go-last.png"));
		ui.hideButton->setText("Show");
	}
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

void MainWindow::closeEvent(QCloseEvent *event)
 {
	settings->setValue("reader_geometry", saveGeometry());
	event->accept();
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
			// Sometimes messages don't have a real subject - only threads do.
			// Check for this:
			if (threadHeaderMessage.subject.length() < thread->name.length())
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
#ifdef Q_WS_HILDON
	hideClickedSlot();
#endif
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
	ui.viewInBrowser->setEnabled(true);
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

void MainWindow::setForumStatus(int forum, bool reloading, float progress) {
	if (reloading) {
		busyForums.insert(forum);
	} else {
		busyForums.remove(forum);
	}
	ui.stopButton->setEnabled(!busyForums.isEmpty() && readerReady);
	ui.updateButton->setEnabled(busyForums.isEmpty() && readerReady);
	if (!busyForums.isEmpty()) {
		ui.statusbar->showMessage("Updating Forums, please stand by.. ", 5000);
	} else {
		ui.statusbar->showMessage("Forums updated", 5000);
	}

	flw->setForumStatus(forum, reloading, progress);
}

ForumListWidget* MainWindow::forumList() {
	return flw;
}

void MainWindow::setReaderReady(bool ready) {
	readerReady = ready;
	ui.updateButton->setEnabled(readerReady);
	flw->setEnabled(readerReady);
	if(!ready) {
		ui.statusbar->showMessage("Starting up, please wait..", 2000);
	} else {
		ui.statusbar->showMessage("Siilihai is ready", 2000);
	}
}
