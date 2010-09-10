#include "threadlistthreaditem.h"

ThreadListThreadItem::ThreadListThreadItem(QTreeWidget *tree, ForumThread *thread) : ThreadListMessageItem(tree)
{
    Q_ASSERT(thread);
    thr = thread;
    showMoreItem = 0;
    treeWidget = tree;
    Q_ASSERT(thread->isSane());
    Q_ASSERT(treeWidget);
    QString threadSubject = thread->name();
    QString lc = thread->lastchange();
    QString author = "";
    QString orderString;
    if(thread->ordernum() >=0) {
        orderString = QString().number(thread->ordernum()).rightJustified(6, '0');
    }
    connect(thr, SIGNAL(destroyed()), this, SLOT(threadDeleted()));
    connect(thr, SIGNAL(changed(ForumThread*)), this, SLOT(updateItem()));
    connect(thr, SIGNAL(unreadCountChanged(ForumThread *)), this, SLOT(updateUnreads()));

    setText(0, threadSubject);
    setText(1, MessageFormatting::sanitize(lc));
    setText(2, MessageFormatting::sanitize(author));
    setText(3, orderString);
}

void ThreadListThreadItem::setMessage(ForumMessage *message) {
    Q_ASSERT(message);
    msg = message;
    connect(msg, SIGNAL(changed(ForumMessage*)), this, SLOT(updateItem()));
    connect(msg, SIGNAL(markedRead(ForumMessage*,bool)), this, SLOT(updateRead()));
    connect(msg, SIGNAL(destroyed()), this, SLOT(threadMessageDeleted()));

    updateItem();
}

ForumThread* ThreadListThreadItem::thread() {
    return thr;
}

void ThreadListThreadItem::updateItem() {
    ThreadListMessageItem::updateItem();

    if(thr->hasMoreMessages() && !showMoreItem) { // Need to add show more-button
        showMoreItem = new ThreadListShowMoreItem(this);
    } else if(!thr->hasMoreMessages() && showMoreItem) { // Need to delete show more-button
        removeChild(showMoreItem);
        delete showMoreItem;
        showMoreItem = 0;
    }
}

void ThreadListThreadItem::updateUnreads() {
    if(!msg) return;
    if(!thr) return;
    int unreads = thr->unreadCount();
    QString threadSubject = messageSubject;
    QString moreString = QString::null;
    if(thread()->hasMoreMessages()) moreString = "+";
    if (unreads) {
        threadSubject += " (" + QString().number(unreads) + moreString + ")";
    } else if(!moreString.isNull()) {
        threadSubject += " (" + moreString + ")";
    }
    setText(0, threadSubject);
}

void ThreadListThreadItem::threadDeleted() {
    ThreadListThreadItem *threadItem = this;
    Q_ASSERT(!((QTreeWidgetItem*)threadItem)->parent()); // Item should always be root item
    if(showMoreItem) {
        removeChild(showMoreItem);
        delete showMoreItem;
        showMoreItem = 0;
    }
    for(int i=((QTreeWidgetItem*)threadItem)->childCount()-1;i >= 0; i--) {
        QTreeWidgetItem *child = ((QTreeWidgetItem*)threadItem)->child(i);
        if(dynamic_cast<ThreadListMessageItem*>(child)){
            removeChild(child);
            delete(child);
        } else {
            Q_ASSERT(false); // Unknown type in thread!
        }
    }
    treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(this));
    deleteLater();
}

void ThreadListThreadItem::threadMessageDeleted() {
    msg = 0;
}
