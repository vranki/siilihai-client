#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
	setColumnCount(3);
	QStringList headers;
	headers << "Subject" << "Date" << "Author";
	setHeaderLabels(headers);
        connect(&fdb, SIGNAL(threadFound(ForumThread*)), this, SLOT(threadFound(ForumThread*)));
        connect(&fdb, SIGNAL(messageFound(ForumMessage*)), this, SLOT(messageFound(ForumMessage*)));
        connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
                this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
}

ThreadListWidget::~ThreadListWidget() {

}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
	clear();
	forumMessages.clear();
	if (!fg) {
		emit messageSelected(0);
		return;
	}
        return;

	QList<QTreeWidgetItem *> items;
	foreach(ForumThread *thread, fdb.listThreads(fg)) {
		QStringList header;
		// @todo messageformatting::sanitize
		header << thread->name() << thread->lastchange() << "author" /*
				<< thread->author()*/;
		QTreeWidgetItem *threadItem = new QTreeWidgetItem(this, header);
		foreach(ForumMessage *message, fdb.listMessages(thread)) {
			QStringList messageHeader;
			messageHeader << message->lastchange() << message->author();
			QTreeWidgetItem *messageItem = new QTreeWidgetItem(threadItem,
					messageHeader);

			threadItem->addChild(messageItem);
			forumMessages[messageItem] = message;
			updateMessageRead(messageItem);
		}
		items.append(threadItem);
	}
	insertTopLevelItems(0, items);
	resizeColumnToContents(0);
	resizeColumnToContents(1);
	resizeColumnToContents(2);
disconnect(this,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
		this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
connect(this,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
		this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
}

void ThreadListWidget::updateMessageRead(QTreeWidgetItem *item) {
	Q_ASSERT(item);
	// if item is a message in a thread, update thread's read count
	QTreeWidgetItem *threadItem = item->parent();
	if(threadItem) {
		updateThreadUnreads(threadItem);
	} else {
		updateThreadUnreads(item);
	}
	QFont font = item->font(0);
	if (forumMessages[item]->read()) {
		font.setBold(false);
		item->setIcon(0, QIcon(":/data/emblem-mail.png"));
	} else {
		font.setBold(true);
		item->setIcon(0, QIcon(":/data/mail-unread.png"));
	}
	item->setFont(0, font);
	if(threadItem)
		updateMessageRead(threadItem);
}

void ThreadListWidget::updateThreadUnreads(QTreeWidgetItem* threadItem) {
	if(threadItem) {
		Q_ASSERT(forumMessages.contains(threadItem));
		ForumMessage *thread = forumMessages[threadItem];
		Q_ASSERT(thread->isSane());
		int unreads = 0;
		if(!forumMessages[threadItem]->read())
			unreads++; // Also count first message
		for(int i=0;i<threadItem->childCount();i++) {
			if(!forumMessages[threadItem->child(i)]->read())
				unreads++;
		}

		QString threadSubject = thread->subject();
		if (unreads) {
			threadSubject += " (" + QString().number(unreads) + ")";
		}
		threadItem->setText(0, threadSubject);
	}
}

void ThreadListWidget::messageSelected(QTreeWidgetItem* item,
		QTreeWidgetItem *prev) {
	if (item == 0)
		return;
	if (!forumMessages.contains(item)) {
		qDebug() << "A thread with no messages. Broken parser?.";
		return;
	}
	ForumMessage *msg = forumMessages[item];
	Q_ASSERT(msg->isSane());
	// forumMessages[item].read = true;
	emit messageSelected(msg);
	updateMessageRead(item);
}

void ThreadListWidget::threadFound(ForumThread* thr) {
    QStringList header;
    // @todo messageformatting::sanitize
    header << thr->name() << thr->lastchange() << "author" /*
                            << thread->author()*/;
    QTreeWidgetItem *threadItem = new QTreeWidgetItem(this, header);
    addTopLevelItem(threadItem);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::messageFound(ForumThread* msg){

}
