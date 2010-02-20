#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
	setColumnCount(3);
        currentGroup = 0;
	QStringList headers;
	headers << "Subject" << "Date" << "Author";
	setHeaderLabels(headers);
        connect(&fdb, SIGNAL(threadFound(ForumThread*)), this, SLOT(threadFound(ForumThread*)));
        connect(&fdb, SIGNAL(messageFound(ForumMessage*)), this, SLOT(messageFound(ForumMessage*)));
        connect(&fdb, SIGNAL(threadDeleted(ForumThread*)), this, SLOT(threadDeleted(ForumThread*)));
        connect(&fdb, SIGNAL(messageDeleted(ForumMessage*)), this, SLOT(messageDeleted(ForumMessage*)));
        connect(&fdb, SIGNAL(messageUpdated(ForumMessage*)), this, SLOT(messageUpdated(ForumMessage*)));
        connect(&fdb, SIGNAL(groupUpdated(ForumGroup*)), this, SLOT(groupUpdated(ForumGroup*)));
        connect(&fdb, SIGNAL(groupDeleted(ForumGroup*)), this, SLOT(groupDeleted(ForumGroup*)));
        connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
                this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
}

ThreadListWidget::~ThreadListWidget() {
}

void ThreadListWidget::messageDeleted(ForumMessage *msg) {
    if(msg->thread()->group() != currentGroup) return;

    QTreeWidgetItem *messageItem = messageWidget(msg);
    Q_ASSERT(messageItem);
    if(messageItem) {
        if(messageItem->parent()) { // Is a standard message
            messageItem->parent()->removeChild(messageItem);
        } else { // Is a thread's first
            takeTopLevelItem(indexOfTopLevelItem(messageItem));
            // @todo i really hope this works if this is deleted first!
        }

        delete messageItem;
    }
}

void ThreadListWidget::threadDeleted(ForumThread *thread) {

}

void ThreadListWidget::groupUpdated(ForumGroup *grp) {
    if(grp != currentGroup) return;
    if(!grp->subscribed()) {
        groupDeleted(grp);
    }
}

void ThreadListWidget::groupDeleted(ForumGroup *grp) {
    if(grp != currentGroup) return;
    groupSelected(0);
}

void ThreadListWidget::messageFound(ForumMessage *msg) {
    if(msg->thread()->group() != currentGroup) return;
    groupSelected(currentGroup); // @todo better
}

void ThreadListWidget::threadFound(ForumThread *thread) {
    if(thread->group() != currentGroup) return;
    groupSelected(currentGroup);  // @todo better
}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
    qDebug() << Q_FUNC_INFO << fg << " old was " << currentGroup;
    Q_ASSERT(fg);
    if(currentGroup == fg) return;
    currentGroup = fg;
    // @todo do nothing if group hasn't changed!
    clear();
    forumMessages.clear();
    messageSubjects.clear();
    if (!fg) {
        emit messageSelected(0);
        return;
    }

    QList<QTreeWidgetItem *> items;
    foreach(ForumThread *thread, fdb.listThreads(fg)) {
        QList<ForumMessage*> messages = fdb.listMessages(thread);
        ForumMessage *firstMessage = 0;
        if(messages.size() > 1) {
            firstMessage = messages.first();
            QString threadSubject = messageSubject(firstMessage);
            QStringList header;
            // @todo messageformatting::sanitize
            QString lc = thread->lastchange();
            QString author = firstMessage->author();
            header << threadSubject << MessageFormatting::sanitize(lc) << MessageFormatting::sanitize(author);

            QTreeWidgetItem *threadItem = new QTreeWidgetItem(this, header);
            forumMessages[threadItem] = firstMessage;
            messageSubjects[threadItem] = threadSubject;
            items.append(threadItem);

            foreach(ForumMessage *message, messages) {
                if(message != firstMessage) {
                    QStringList messageHeader;
                    QString subject = messageSubject(message);
                    if(subject == threadSubject) {
                        subject = "Re: " + threadSubject;
                    }
                    lc = message->lastchange();
                    author = message->author();
                    messageHeader << subject << MessageFormatting::sanitize(lc) << MessageFormatting::sanitize(author);
                    QTreeWidgetItem *messageItem = new QTreeWidgetItem(threadItem,
                                                                       messageHeader);
                    threadItem->addChild(messageItem);
                    forumMessages[messageItem] = message;
                    messageSubjects[messageItem] = subject;
                    updateMessageRead(messageItem);
                }
            }
        } else {
            qDebug() << "Warning: Thread " << thread->toString() << " contains no messages!";
        }
    }
    insertTopLevelItems(0, items);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

QString ThreadListWidget::messageSubject(ForumMessage *msg) {
    QString subj;
    if(msg->subject().length() > msg->thread()->name().length()) {
        subj = msg->subject();
    } else {
        subj = msg->thread()->name();
    }
    subj = MessageFormatting::sanitize(subj);
    return subj;
}

void ThreadListWidget::messageUpdated(ForumMessage *msg) {
    if(msg->thread()->group() != currentGroup) return;
    QTreeWidgetItem *twi = messageWidget(msg);
    if(twi) {
        updateMessageRead(messageWidget(msg));
    } else {
        qDebug() << Q_FUNC_INFO << "Message updated but i don't have it: " << msg->toString();
    }
}

QTreeWidgetItem* ThreadListWidget::messageWidget(ForumMessage *msg) {
    foreach(QTreeWidgetItem *twi, forumMessages.keys()) {
        if(forumMessages[twi] == msg) {
            return twi;
        }
    }
    return 0;
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
        ForumMessage *thread = forumMessages[threadItem];
        Q_ASSERT(thread);
        int unreads = 0;
        if(!forumMessages[threadItem]->read())
            unreads++; // Also count first message
        for(int i=0;i<threadItem->childCount();i++) {
            if(!forumMessages[threadItem->child(i)]->read())
                unreads++;
        }

        QString threadSubject = messageSubjects[threadItem];
        if (unreads) {
            threadSubject += " (" + QString().number(unreads) + ")";
        }
        threadItem->setText(0, threadSubject);
    }
}

void ThreadListWidget::messageSelected(QTreeWidgetItem* item,
		QTreeWidgetItem *prev) {
    qDebug() << "selected " << item << prev;
    Q_UNUSED(prev);
    if (!item)
        return;
    if (!forumMessages.contains(item)) {
        qDebug() << "A thread with no messages. Broken parser?.";
        return;
    }
    ForumMessage *msg = forumMessages[item];
    Q_ASSERT(msg->isSane());
    emit messageSelected(msg);
    updateMessageRead(item);
}
