#include "threadlistwidget.h"

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
    setColumnCount(3);
    currentGroup = 0;
    disableSortAndResize = false;
    QStringList headers;
    headers << "Subject" << "Date" << "Author" << "Ordernum";
    setHeaderLabels(headers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(&fdb, SIGNAL(messageFound(ForumMessage*)), this, SLOT(messageFound(ForumMessage*)));
    connect(&fdb, SIGNAL(threadFound(ForumThread*)), this, SLOT(threadFound(ForumThread*)));

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
    forceUpdateThreadAction = new QAction("Force update of thread", this);
    forceUpdateThreadAction->setToolTip("Updates all messages in selected thread");
    connect(forceUpdateThreadAction, SIGNAL(triggered()), this, SLOT(forceUpdateThreadClicked()));
}

ThreadListWidget::~ThreadListWidget() {
}

void ThreadListWidget::groupChanged(ForumGroup *grp) {
    if(grp != currentGroup) return;

    if(!grp->subscribed()) {
        groupDeleted(grp);
    }
}

void ThreadListWidget::groupDeleted(QObject*g) {
    ForumGroup *grp = static_cast<ForumGroup*>(g);
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
    // qDebug() << Q_FUNC_INFO << message->toString();
    QPair<QTreeWidgetItem*, ForumThread*> threadPair;

    ThreadListThreadItem *threadItem = forumThreads.key(message->thread());
    Q_ASSERT(threadItem);

    ThreadListMessageItem *item = 0;
    if(message->ordernum() == 0) { // First message - update thread item!
        item = threadItem;
        threadItem->setMessage(message);
        // qDebug() << Q_FUNC_INFO << "setting the thread item";
    } else { // Reply message - create new item
        item = new ThreadListMessageItem(threadItem, message);
    }

    forumMessages[item] = message;
    item->updateItem();
    item->updateRead();

    // Update the thread item
    if(!dynamic_cast<ThreadListThreadItem*> (item)) {
        forumThreads.key(message->thread())->updateUnreads();
    }
    if(!disableSortAndResize) {
        sortItems(3, Qt::AscendingOrder);
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);
    }
}

void ThreadListWidget::addThread(ForumThread *thread) {
    Q_ASSERT(thread);
    Q_ASSERT(thread->group() == currentGroup);
    // qDebug() << Q_FUNC_INFO << thread->toString();

    ThreadListThreadItem *threadItem = new ThreadListThreadItem(this, thread);

    forumThreads[threadItem] = thread;
    addTopLevelItem(threadItem);
    if(!disableSortAndResize) {
        sortItems(3, Qt::AscendingOrder);
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);
    }
}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
    if(!fg) {
        if(currentGroup) {
            disconnect(currentGroup, SIGNAL(changed(ForumGroup*)), this, SLOT(groupChanged(ForumGroup*)));
            currentGroup = 0;
        }
        clearList();
        emit messageSelected(0);
    }
    if(currentGroup != fg) {
        currentGroup = fg;
        connect(currentGroup, SIGNAL(changed(ForumGroup*)), this, SLOT(groupChanged(ForumGroup*)));
        connect(currentGroup, SIGNAL(destroyed(QObject*)), this, SLOT(groupDeleted(QObject*)));
        clearSelection();
        updateList();
        setCurrentItem(topLevelItem(0));
    }
}

void ThreadListWidget::clearList() {
    forumMessages.clear();
    forumThreads.clear();
    clear();
}

void ThreadListWidget::updateList() {
    if(!currentGroup) return;
    clearList();
    // Add the threads and messages in order
    QList<ForumThread*> threads = currentGroup->threads().values();
    qSort(threads);
    disableSortAndResize = true;
    foreach(ForumThread *thread, threads) {
        addThread(thread);
        QList<ForumMessage*> messages = thread->values();
        qSort(messages);
        foreach(ForumMessage *message, messages) {
            addMessage(message);
        }
    }
    disableSortAndResize = false;
    sortItems(3, Qt::AscendingOrder);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev) {
    Q_UNUSED(prev);
    if (!item)
        return;
    if(dynamic_cast<ThreadListShowMoreItem*> (item)) {
        ThreadListShowMoreItem * smItem = dynamic_cast<ThreadListShowMoreItem*> (item);
        ThreadListThreadItem* tli = dynamic_cast<ThreadListThreadItem*> (smItem->parent());
        setCurrentItem(prev);
        emit moreMessagesRequested(tli->thread());
    } else if (dynamic_cast<ThreadListMessageItem*> (item)) {
        ThreadListMessageItem* msgItem = dynamic_cast<ThreadListMessageItem*> (item);
        ForumMessage *msg = msgItem->message();
        if(!msg) {
            qDebug() << Q_FUNC_INFO << "Thread item with no message? Broken parser?";
        } else {
            Q_ASSERT(msg);
            Q_ASSERT(msg->isSane());
            emit messageSelected(msg);
            msgItem->updateRead();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "A thread with no messages. Broken parser?.";
        //if(forumThreads.contains(item))
        //    qDebug() << "The thread in question is " << forumThreads.value(item)->toString();
        emit messageSelected(0);
    }
}

void ThreadListWidget::contextMenuEvent(QContextMenuEvent *event) {
    if(itemAt(event->pos())) {
        QMenu menu(this);
        menu.addAction(viewInBrowserAction);
        menu.addAction(markReadAction);
        menu.addAction(markUnreadAction);
        menu.addAction(forceUpdateThreadAction);
        menu.addAction(threadPropertiesAction);
        menu.exec(event->globalPos());
    }
}

void ThreadListWidget::markReadClicked(bool read) {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    ForumMessage *threadMessage = msgItem->message();
    if(threadMessage) {
        foreach(ForumMessage *msg, threadMessage->thread()->values()) {
            msg->setRead(read);
        }
    }
}

void ThreadListWidget::markUnreadClicked() {
    markReadClicked(false);
}

void ThreadListWidget::threadPropertiesClicked() {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    ForumMessage *threadMessage = msgItem->message();
    if(threadMessage) {
        emit threadProperties(threadMessage->thread());
    }
}

void ThreadListWidget::viewInBrowserClicked() {
    emit viewInBrowser();
}

void ThreadListWidget::selectNextUnread() {
    QTreeWidgetItem *newItem = currentItem();
    ThreadListMessageItem *mi = 0;
    while(newItem) {
        if(newItem->childCount()) { // Thread item
            newItem = newItem->child(0);
        } else if(newItem->parent()) { // A message item
            QTreeWidgetItem *nextChild = newItem->parent()->child(newItem->parent()->indexOfChild(newItem) + 1);
            if(!nextChild) { // Jump to next parent
                newItem = topLevelItem(indexOfTopLevelItem(newItem->parent()) + 1);
                if(!newItem) return; // No more threads to search
            } else {
                newItem = nextChild;
            }
        } else { // Single message thread
            newItem = topLevelItem(indexOfTopLevelItem(newItem) + 1);
            if(!newItem) return; // No more threads to search
        }
        ThreadListMessageItem *mi = dynamic_cast<ThreadListMessageItem*> (newItem);
        if(mi && mi->message()) {
            if(!mi->message()->isRead()) {
                setCurrentItem(newItem);
                return;
            }
        }
    }
    /*
    QTreeWidgetItem *newItem = 0;
    ThreadListMessageItem *mi = 0;
    bool isShowMore = false;

    // Find next unread item
    if(item) {
        bool isRead = true;
        do {
            if(item->childCount()) {
                // Is a thread item with child
                newItem = item->child(0);
            } else if(item->parent()) {
                // Is a message item
                QTreeWidgetItem *nextItem = item->parent()->child(item->parent()->indexOfChild(item) + 1);
                if(nextItem) {
                    // ..and has a next item after it
                    newItem = nextItem;
                } else {
                    // ..and is last in thread
                    newItem = topLevelItem(indexOfTopLevelItem(item->parent()) + 1);
                }
            } else {
                // Is a thread item without child
                qDebug() << Q_FUNC_INFO << "Top level item without child";
                newItem = 0;
                if(indexOfTopLevelItem(item) + 1 < topLevelItemCount())
                    newItem = topLevelItem(indexOfTopLevelItem(item) + 1);
            }
            item = newItem;
            if(item) {
                mi = dynamic_cast<ThreadListMessageItem*> (newItem);
                isShowMore = dynamic_cast<ThreadListShowMoreItem*> (newItem);
                isRead = true;
                if(mi && mi->message() && mi->message())
                    isRead = mi->message()->isRead();
                ForumMessage *message = mi->message(); // Shouldn't be 0 but sometimes is
            }
        } while(item && (isShowMore || isRead));
        if(mi) setCurrentItem(mi);
    }
    */
}
void ThreadListWidget::forceUpdateThreadClicked() {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    if(msgItem && msgItem->message() && msgItem->message()->thread()) {
        msgItem->message()->thread()->setLastPage(0);
        emit updateThread(msgItem->message()->thread(), true);
    }
}
