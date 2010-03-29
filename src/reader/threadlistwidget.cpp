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

    QTreeWidgetItem *messageItem = messageWidget(msg);
    // Q_ASSERT(messageItem); should exist always?
    if(messageItem) {
        if(messageItem->parent()) { // Is a standard message
            messageItem->parent()->removeChild(messageItem);
        } else { // Is a thread's first
            takeTopLevelItem(indexOfTopLevelItem(messageItem));
            // @todo i really hope this works if this is deleted first!
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
    QPair<QTreeWidgetItem*, ForumThread*> threadPair;

    QTreeWidgetItem *threadItem = threadWidget(message->thread());
    Q_ASSERT(threadItem);

    QStringList messageHeader;
    QString subject = messageSubject(message);
    QString lc = message->lastchange();
    lc = MessageFormatting::sanitize(lc);
    QString author = message->author();
    author = MessageFormatting::sanitize(author);
    QString orderString = QString().number(message->ordernum());
    while(orderString.length() < 4) orderString = "0"+orderString;

    QTreeWidgetItem *item = 0;
    if(message->ordernum()==0) { // First message - update thread item!
        item = threadItem;
    } else { // Reply message - create new item
        item = new QTreeWidgetItem(threadItem);
        item->setText(3, orderString);
    }
    item->setText(0, subject);
    item->setText(1, lc);
    item->setText(2, author);

    forumMessages[item] = message;
    messageSubjects[item] = subject;

    updateMessageRead(item);
}

void ThreadListWidget::addThread(ForumThread *thread) {
    Q_ASSERT(thread);
    QString threadSubject = thread->name();//messageSubject(thread->name());
    QString lc = thread->lastchange();
    QString author = "";
    QString orderString = QString().number(thread->ordernum());
    while(orderString.length() < 4) orderString = "0"+orderString;

    QTreeWidgetItem *threadItem = new QTreeWidgetItem(this);
    threadItem->setText(0, threadSubject);
    threadItem->setText(1, MessageFormatting::sanitize(lc));
    threadItem->setText(2, MessageFormatting::sanitize(author));
    threadItem->setText(3, orderString);

    forumThreads[threadItem] = thread;
    addTopLevelItem(threadItem);
    sortItems(3, Qt::AscendingOrder);
}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
    if(!fg) {
        currentGroup = fg;
        clearList();
        emit messageSelected(0);
    } if(currentGroup != fg) {
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
    ForumMessage *message = forumMessages[item];
    // message is 0 if a thread doesn't yet have its first message.
    if(!message) return;

    QTreeWidgetItem *parentItem = item->parent();
    if(parentItem) {
        updateThreadUnreads(parentItem);
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
    if(parentItem)
        updateMessageRead(parentItem);
}

void ThreadListWidget::updateThreadUnreads(QTreeWidgetItem* threadItem) {
    if(threadItem) {
        ForumMessage *message = forumMessages[threadItem];
        // message is 0 if a thread doesn't yet have its first message.
        if(!message) return;
        int unreads = 0;
        if(!message->read())
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
    qDebug() << Q_FUNC_INFO << "selected " << item << prev;
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
