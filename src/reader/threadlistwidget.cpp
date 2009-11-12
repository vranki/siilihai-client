#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) : QTreeWidget(parent), fdb(f) {
	setColumnCount(3);
	QStringList headers;
	headers << "Subject" << "Date" << "Author";
	setHeaderLabels(headers);
}

ThreadListWidget::~ThreadListWidget() {

}

void ThreadListWidget::groupSelected(ForumGroup fg) {
	clear();
	forumMessages.clear();
	if (!fg.isSane())
		return;

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
		threadHeaderMessage.subject = MessageFormatting::sanitize(threadHeaderMessage.subject);
		threadHeaderMessage.author = MessageFormatting::sanitize(threadHeaderMessage.author);
		threadHeaderMessage.lastchange = MessageFormatting::sanitize(threadHeaderMessage.lastchange);

		header << threadHeaderMessage.subject << threadHeaderMessage.lastchange
				<< threadHeaderMessage.author;

		QTreeWidgetItem *threadItem =
				new QTreeWidgetItem(this, header);
		if (messages.size() > 0) {
			forumMessages[threadItem] = messages[0];

			for (int m = 1; m < messages.size(); ++m) {
				ForumMessage *message = &messages[m];
				Q_ASSERT(message->isSane());
				QStringList messageHeader;
				message->subject = MessageFormatting::sanitize(message->subject);
				message->author = MessageFormatting::sanitize(message->author);
				message->lastchange = MessageFormatting::sanitize(message->lastchange);

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

void ThreadListWidget::messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev) {
	if (item == 0)
		return;
	if (!forumMessages.contains(item)) {
		qDebug() << "A thread with no messages. Broken parser?.";
		return;
	}

	ForumMessage *msg = &forumMessages[item];
	Q_ASSERT(msg->isSane());
	forumMessages[item].read = true;
	emit messageSelected(*msg);
	updateMessageRead(item);
}
