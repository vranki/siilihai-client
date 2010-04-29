#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
    setColumnCount(3);
    currentGroup = 0;
    QStringList headers;
    headers << "Subject" << "Date" << "Author" << "Ordernum";
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
    hideColumn(3);
}

ThreadListWidget::~ThreadListWidget() {
}

void ThreadListWidget::messageDeleted(ForumMessage *msg) {
    Q_ASSERT(msg);
    if(msg->thread()->group() != currentGroup) return;
    qDebug() << Q_FUNC_INFO << msg->toString();

    QTreeWidgetItem *messageItem = messageWidget(msg);
    // Q_ASSERT(messageItem); should exist always?
    if(messageItem) {
        if(messageItem->parent()) { // Is a standard message
            messageItem->parent()->removeChild(messageItem);
        } else { // Is a thread's first
            // Remove child items
            QList<QTreeWidgetItem *> childItems = messageItem->takeChildren();
            // Remove the item itself
            takeTopLevelItem(indexOfTopLevelItem(messageItem));

            QTreeWidgetItem *newThreadFirstItem = 0;
            // Find out which is the next first message in thread:
            QList<ForumMessage*> messages = fdb.listMessages(msg->thread());
            foreach(ForumMessage* message, messages) {
                qDebug() << "Checking for next msg " << message << " on= " << message->ordernum() << ", wanted:" << msg->ordernum()+1;
                if(message->ordernum() <= msg->ordernum() + 1) {
                    newThreadFirstItem = messageWidget(message);
                    qDebug() << "..Selected this";
                }
            }
            if(newThreadFirstItem) {
                addTopLevelItem(newThreadFirstItem);
                forumThreads[newThreadFirstItem] = msg->thread();
            }
            Q_ASSERT(newThreadFirstItem);
            foreach(ForumMessage* message, messages) {
                if(message->ordernum() > msg->ordernum() + 1){
                    // Add as child
                    QTreeWidgetItem *childItem = messageWidget(message);
                    Q_ASSERT(childItem);
                    newThreadFirstItem->addChild(childItem);
                }
            }
        }
        forumMessages.remove(messageItem);
        messageSubjects.remove(messageItem);
        delete messageItem;
    } else {
        qDebug() << Q_FUNC_INFO << "Message item not found for some reason for " << msg->toString();
    }
}

void ThreadListWidget::threadDeleted(ForumThread *thread) {
    if(thread->group() != currentGroup) return;

    // Actually NOP, as deleting the last message will remove the thread.
    // @todo check that all messages have been removed here!
    return;

    QTreeWidgetItem *threadItem = threadWidget(thread);
    // Q_ASSERT(messageItem); should exist always?
    if(threadItem) {
        if(threadItem->parent()) { // Is a standard message
            threadItem->parent()->removeChild(threadItem);
        } else { // Is a thread's first
            takeTopLevelItem(indexOfTopLevelItem(threadItem));
            // @todo i really hope this works if this is deleted first!
        }
        forumMessages.remove(threadItem);
        messageSubjects.remove(threadItem);
        // delete threadItem; NOT needed, as messageDeleted() deletes the item when first msg is deleted!
    } else {
        qDebug() << Q_FUNC_INFO << "Message item not found for some reason for " << thread->toString();
    }
}

void ThreadListWidget::groupUpdated(ForumGroup *grp) {
    if(grp != currentGroup) return;

    if(!grp->subscribed()) {
        groupDeleted(grp);
    }
}

void ThreadListWidget::groupDeleted(ForumGroup *grp) {
    if(grp == currentGroup)
        groupSelected(0);
}

void ThreadListWidget::messageFound(ForumMessage *msg) {
    if(msg->thread()->group() == currentGroup) addMessage(msg);
}

void ThreadListWidget::threadFound(ForumThread *thread) {
    if(thread->group() == currentGroup) addThread(thread);
}

void ThreadListWidget::addMessage(ForumMessage *message) {
    Q_ASSERT(message);
    qDebug() << Q_FUNC_INFO << message->toString();
    QPair<QTreeWidgetItem*, ForumThread*> threadPair;

    QTreeWidgetItem *threadItem = threadWidget(message->thread());
    Q_ASSERT(threadItem);

    QString orderString;
    if(message->ordernum() >=0) {
        orderString = QString().number(message->ordernum()).rightJustified(4, '0');
    }

    QTreeWidgetItem *item = 0;
    if(message->ordernum() == 0) { // First message - update thread item!
        item = threadItem;
        qDebug() << Q_FUNC_INFO << "setting the thread item";
    } else { // Reply message - create new item
        item = new QTreeWidgetItem(threadItem);
        item->setText(3, orderString);
        qDebug() << Q_FUNC_INFO << "created as new item";
    }

    forumMessages[item] = message;

    updateMessageItem(item, message);
    updateMessageRead(item);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::addThread(ForumThread *thread) {
    Q_ASSERT(thread);
    qDebug() << Q_FUNC_INFO << thread->toString();
    QString threadSubject = thread->name();//messageSubject(thread->name());
    QString lc = thread->lastchange();
    QString author = "";
    QString orderString;
    if(thread->ordernum() >=0) {
        orderString = QString().number(thread->ordernum()).rightJustified(4, '0');
    }

    QTreeWidgetItem *threadItem = new QTreeWidgetItem(this);
    threadItem->setText(0, threadSubject);
    threadItem->setText(1, MessageFormatting::sanitize(lc));
    threadItem->setText(2, MessageFormatting::sanitize(author));
    threadItem->setText(3, orderString);

    forumThreads[threadItem] = thread;
    addTopLevelItem(threadItem);
    sortItems(3, Qt::AscendingOrder);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
    if(!fg) {
        currentGroup = 0;
        clearList();
        emit messageSelected(0);
    }
    if(currentGroup != fg) {
        currentGroup = fg;
        updateList();
    }
}

void ThreadListWidget::clearList() {
    clear();
    forumMessages.clear();
    messageSubjects.clear();
    forumThreads.clear();
}

void ThreadListWidget::updateList() {
    if(!currentGroup) return;
    clearList();

    QList<QTreeWidgetItem *> items;
    foreach(ForumThread *thread, fdb.listThreads(currentGroup)) {
        addThread(thread);
        foreach(ForumMessage *message, fdb.listMessages(thread)) {
            addMessage(message);
        }
    }
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
        if(msg->ordernum() > 0) subj = "Re: " + subj;
    }
    subj = MessageFormatting::sanitize(subj);
    return subj;
}

void ThreadListWidget::messageUpdated(ForumMessage *msg) {
    if(msg->thread()->group() != currentGroup) return;

    QTreeWidgetItem *twi = messageWidget(msg);
    if(twi) {
        if(msg->ordernum() == 0) {
            qDebug() << Q_FUNC_INFO << "Horror! Must swap message with thread header!";
            QTreeWidgetItem *threadItem = threadWidget(msg->thread());
            Q_ASSERT(threadItem);
            ForumMessage *oldThreadMessage = forumMessages[threadItem];
            if(!oldThreadMessage) {
                // Ok, weird situation but there is no thread message (yet)
                // Just set this message as thread message
                oldThreadMessage = msg;
                forumMessages[threadItem] = msg;
            } else {
                swapMessages(msg, oldThreadMessage);
            }
            updateMessageRead(messageWidget(oldThreadMessage));
            updateMessageItem(messageWidget(oldThreadMessage), oldThreadMessage);
        }
        updateMessageRead(messageWidget(msg));
        updateMessageItem(messageWidget(msg), msg);
    } else {
        qDebug() << Q_FUNC_INFO << "Message updated but i don't have it: " << msg->toString();
    }
}

QTreeWidgetItem* ThreadListWidget::messageWidget(ForumMessage *msg) {
    foreach(QTreeWidgetItem *twi, forumMessages.keys()) {
        if(forumMessages.value(twi) == msg) {
            return twi;
        }
    }
    return 0;
}

QTreeWidgetItem* ThreadListWidget::threadWidget(ForumThread *thread) {
    foreach(QTreeWidgetItem *twi, forumThreads.keys()) {
        if(forumThreads[twi] == thread) {
            return twi;
        }
    }
    return 0;
}

void ThreadListWidget::updateMessageRead(QTreeWidgetItem *item) {
    Q_ASSERT(item);
    // if item is a message in a thread, update thread's read count
    ForumMessage *message = forumMessages.value(item);
    // message is 0 if a thread doesn't yet have its first message.
    if(!message) return;

    QTreeWidgetItem *parentItem = item->parent();
    if(parentItem) {
        updateThreadUnreads(parentItem);
    } else {
        updateThreadUnreads(item);
    }
    QFont font = item->font(0);
    if (message->read()) {
        font.setBold(false);
        item->setIcon(0, QIcon(":/data/emblem-mail.png"));
    } else {
        font.setBold(true);
        item->setIcon(0, QIcon(":/data/mail-unread.png"));
    }
    item->setFont(0, font);
    if(parentItem)
        updateMessageRead(parentItem);
}

void ThreadListWidget::updateThreadUnreads(QTreeWidgetItem* threadItem) {
    if(threadItem) {

        ForumMessage *message = forumMessages.value(threadItem);
        // message is 0 if a thread doesn't yet have its first message.
        if(!message) return;
        int unreads = 0;
        if(!message->read())
            unreads++; // Also count first message

        for(int i=0;i<threadItem->childCount();i++) {
            if(!forumMessages.value(threadItem->child(i))->read())
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
    qDebug() << Q_FUNC_INFO << "selected " << item << prev;
    Q_UNUSED(prev);
    if (!item)
        return;
    if (!forumMessages.contains(item)) {
        qDebug() << "A thread with no messages. Broken parser?.";
        if(forumThreads.contains(item))
            qDebug() << "The thread in question is " << forumThreads.value(item)->toString();
        emit messageSelected(0);
        return;
    }
    ForumMessage *msg = forumMessages.value(item);
    Q_ASSERT(msg);
    Q_ASSERT(msg->isSane());
    emit messageSelected(msg);
    updateMessageRead(item);
}

void ThreadListWidget::updateMessageItem(QTreeWidgetItem *item, ForumMessage *message) {
    QString orderString;
    // Orderstring is thread's order if first message, or messages if not:
    if(message->ordernum() == 0) {
        orderString = QString().number(message->thread()->ordernum()).rightJustified(4, '0');
    } else if(message->ordernum() > 0) {
        orderString = QString().number(message->ordernum()).rightJustified(4, '0');
    }
    QString oldOrderString = item->text(3);
    item->setText(3, orderString);

    QString subject = messageSubject(message);
    QString lc = message->lastchange();
    lc = MessageFormatting::sanitize(lc);
    QString author = message->author();
    author = MessageFormatting::sanitize(author);
    item->setText(0, subject);
    item->setText(1, lc);
    item->setText(2, author);
    messageSubjects[item] = subject;

    if(oldOrderString != orderString)
        sortItems(3, Qt::AscendingOrder);
}

void ThreadListWidget::swapMessages(ForumMessage *m1, ForumMessage *m2) {
    qDebug() << Q_FUNC_INFO << "Swapping " << m1->toString() << " with " << m2->toString();

    QTreeWidgetItem *i1 = messageWidget(m1);
    QTreeWidgetItem *i2 = messageWidget(m2);

    forumMessages[i1] = m2;
    forumMessages[i2] = m1;
}

