#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
    setColumnCount(3);
    currentGroup = 0;
    QStringList headers;
    headers << "Subject" << "Date" << "Author" << "Ordernum";
    setHeaderLabels(headers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(&fdb, SIGNAL(messageFound(ForumMessage*)), this, SLOT(messageFound(ForumMessage*)));
    connect(&fdb, SIGNAL(threadFound(ForumThread*)), this, SLOT(threadFound(ForumThread*)));
    connect(&fdb, SIGNAL(threadUpdated(ForumThread*)), this, SLOT(threadUpdated(ForumThread*)));
    connect(&fdb, SIGNAL(threadDeleted(ForumThread*)), this, SLOT(threadDeleted(ForumThread*)));
    connect(&fdb, SIGNAL(messageDeleted(ForumMessage*)), this, SLOT(messageDeleted(ForumMessage*)));
    connect(&fdb, SIGNAL(messageUpdated(ForumMessage*)), this, SLOT(messageUpdated(ForumMessage*)));
    connect(&fdb, SIGNAL(groupUpdated(ForumGroup*)), this, SLOT(groupUpdated(ForumGroup*)));
    connect(&fdb, SIGNAL(groupDeleted(ForumGroup*)), this, SLOT(groupDeleted(ForumGroup*)));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
            this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
    hideColumn(3);

    markReadAction = new QAction("Mark thread read", this);
    markReadAction->setToolTip("Marks all messages in this thread read");
    connect(markReadAction, SIGNAL(triggered()), this, SLOT(markReadClicked()));
    markUnreadAction = new QAction("Mark thread unread", this);
    markUnreadAction->setToolTip("Marks all messages in this thread as unread");
    connect(markUnreadAction, SIGNAL(triggered()), this, SLOT(markUnreadClicked()));
    threadPropertiesAction = new QAction("Thread properties", this);
    threadPropertiesAction->setToolTip("Information and settings for selected thread");
    connect(threadPropertiesAction, SIGNAL(triggered()), this, SLOT(threadPropertiesClicked()));
    viewInBrowserAction = new QAction("View in browser", this);
    viewInBrowserAction->setToolTip("View the message in external browser");
    connect(viewInBrowserAction, SIGNAL(triggered()), this, SLOT(viewInBrowserClicked()));
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

void ThreadListWidget::threadUpdated(ForumThread *thread) {
    if(thread->group() != currentGroup) return;
    // Find if we have show more button item:
    QTreeWidgetItem *item = 0;
    foreach(ForumThread *shThread, showMoreItems.values()) {
        if(shThread == thread) {
            item = showMoreItems.key(shThread);
        }
    }

    if(thread->hasMoreMessages() && !item) { // Need to add show more-button
        addShowMoreButton(thread);
    } else if(!thread->hasMoreMessages() && item) { // Need to delete show more-button
        if(!item->parent()) {
            qDebug() << Q_FUNC_INFO << "WTF!! Found a show more button WITHOUT parent!";
            qDebug() << item->text(0) << item->text(1) << item->text(2) << item->text(3);
        }
        Q_ASSERT(item->parent());
        item->parent()->removeChild(item);
        delete item;
        showMoreItems.remove(item);
        sortItems(3, Qt::AscendingOrder);
    }
    // @todo update other thread fields such as subject etc
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::addShowMoreButton(ForumThread *thread) {
    // Add the show more-item
    QTreeWidgetItem *threadItem = threadWidget(thread);
    Q_ASSERT(threadItem);
    QTreeWidgetItem *showMoreItem = new QTreeWidgetItem(threadItem);
    showMoreItem->setText(0, "Show More messages");
    showMoreItem->setText(3, "999999"); // Always the last one
    showMoreItems[showMoreItem] = thread;
    Q_ASSERT(showMoreItems[showMoreItem]->parent());
    sortItems(3, Qt::AscendingOrder);
}

void ThreadListWidget::threadDeleted(ForumThread *thread) {
    if(thread->group() != currentGroup) return;
    // Remeber, this is recursive ie. deletes messages also!

    QTreeWidgetItem *threadItem = threadWidget(thread);
    if(threadItem) {
        Q_ASSERT(!threadItem->parent()); // Item should always be root item
        for(int i=threadItem->childCount()-1;i >= 0; i--) {
            QTreeWidgetItem *child = threadItem->child(i);
            if(showMoreItems.contains(child)) {
                showMoreItems.remove(child);
            } else {
                messageDeleted(forumMessages.value(child));
                QCoreApplication::processEvents(); // Keep UI responsive
            }
        }
        takeTopLevelItem(indexOfTopLevelItem(threadItem));
        forumThreads.remove(threadItem);
        messageSubjects.remove(threadItem);
        delete threadItem;
    } else {
        qDebug() << Q_FUNC_INFO << "Thread item not found for some reason for " << thread->toString();
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
    Q_ASSERT(message->thread()->group() == currentGroup);
    qDebug() << Q_FUNC_INFO << message->toString();
    QPair<QTreeWidgetItem*, ForumThread*> threadPair;

    QTreeWidgetItem *threadItem = threadWidget(message->thread());
    Q_ASSERT(threadItem);

    QString orderString;
    if(message->ordernum() >=0) {
        orderString = QString().number(message->ordernum()).rightJustified(6, '0');
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
    sortItems(3, Qt::AscendingOrder);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::addThread(ForumThread *thread) {
    Q_ASSERT(thread);
    Q_ASSERT(thread->group() == currentGroup);
    qDebug() << Q_FUNC_INFO << thread->toString();
    QString threadSubject = thread->name();//messageSubject(thread->name());
    QString lc = thread->lastchange();
    QString author = "";
    QString orderString;
    if(thread->ordernum() >=0) {
        orderString = QString().number(thread->ordernum()).rightJustified(6, '0');
    }

    QTreeWidgetItem *threadItem = new QTreeWidgetItem(this);
    threadItem->setText(0, threadSubject);
    threadItem->setText(1, MessageFormatting::sanitize(lc));
    threadItem->setText(2, MessageFormatting::sanitize(author));
    threadItem->setText(3, orderString);

    forumThreads[threadItem] = thread;
    addTopLevelItem(threadItem);
    if(thread->hasMoreMessages()) {
        addShowMoreButton(thread);
    }

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
        clearSelection();
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
        // Check if message is first AND it is not as thread message
        if(msg->ordernum() == 0 && threadWidget(msg->thread()) != twi) {
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
    return forumThreads.key(thread);
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
            ForumMessage *msg = forumMessages.value(threadItem->child(i));
            // msg is 0 if the child is "Show more" button etc
            if(msg && !msg->read())
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
    Q_UNUSED(prev);
    if (!item)
        return;
    if(showMoreItems.contains(item)) {
        item->parent()->removeChild(item);
        delete item;
        ForumThread *thread = showMoreItems.value(item);
        // Select the last message in thread (hope this works always)
        clearSelection();
        emit moreMessagesRequested(thread);
        return;
    }
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
        orderString = QString().number(message->thread()->ordernum()).rightJustified(6, '0');
    } else if(message->ordernum() > 0) {
        orderString = QString().number(message->ordernum()).rightJustified(6, '0');
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

    Q_ASSERT(!showMoreItems.value(i1));
    Q_ASSERT(!showMoreItems.value(i2));

    forumMessages[i1] = m2;
    forumMessages[i2] = m1;
}


void ThreadListWidget::contextMenuEvent(QContextMenuEvent *event) {
    if(itemAt(event->pos())) {
        QMenu menu(this);
        menu.addAction(viewInBrowserAction);
        menu.addAction(markReadAction);
        menu.addAction(markUnreadAction);
        menu.addAction(threadPropertiesAction);
        menu.exec(event->globalPos());
    }
}

void ThreadListWidget::markReadClicked(bool read) {
    ForumMessage *threadMessage = forumMessages.value(currentItem());
    if(threadMessage) {
        foreach(ForumMessage *msg, fdb.listMessages(threadMessage->thread())) {
            if(msg->read() != read) {
                fdb.markMessageRead(msg, read);
                QCoreApplication::processEvents();
            }
        }
    }
}

void ThreadListWidget::markUnreadClicked() {
    markReadClicked(false);
}

void ThreadListWidget::threadPropertiesClicked() {
    ForumMessage *threadMessage = forumMessages.value(currentItem());
    if(threadMessage) {
        emit threadProperties(threadMessage->thread());
    }
}

void ThreadListWidget::viewInBrowserClicked() {
    emit viewInBrowser();
}
