#include "threadlistthreaditem.h"

ThreadListThreadItem::ThreadListThreadItem(QTreeWidget *tree, ForumThread *itemThread) : ThreadListMessageItem(tree)
{
    Q_ASSERT(itemThread);
    _thread = itemThread;
    showMoreItem = 0;
    treeWidget = tree;
    Q_ASSERT(_thread->isSane());
    Q_ASSERT(treeWidget);
    QString threadSubject = _thread->name();
    QString lc = _thread->lastchange();
    QString author = "";
    QString orderString;
    if(_thread->ordernum() >=0) {
        orderString = QString().number(_thread->ordernum()).rightJustified(6, '0');
    }
    connect(_thread, SIGNAL(destroyed()), this, SLOT(threadDeleted()));
    connect(_thread, SIGNAL(changed(ForumThread*)), this, SLOT(updateItem()));
    connect(_thread, SIGNAL(unreadCountChanged(ForumThread *)), this, SLOT(unreadCountChanged(ForumThread *)));
    connect(_thread, SIGNAL(messageAdded(ForumMessage*)), this, SLOT(addMessage(ForumMessage*)));
    connect(_thread, SIGNAL(messageRemoved(ForumMessage*)), this, SLOT(removeMessage(ForumMessage*)));

    setText(0, threadSubject);
    setText(1, MessageFormatting::sanitize(lc));
    setText(2, MessageFormatting::sanitize(author));
    setText(3, orderString);

    QList<ForumMessage*> messages = _thread->values();
    qSort(messages);
    foreach(ForumMessage *msg, messages) {
        addMessage(msg);
    }
    updateItem();
}

void ThreadListThreadItem::setMessage(ForumMessage *message) {
    msg = message;
    if(message) {
        connect(msg, SIGNAL(changed(ForumMessage*)), this, SLOT(updateItem()));
        connect(msg, SIGNAL(markedRead(ForumMessage*,bool)), this, SLOT(updateRead()));
        connect(msg, SIGNAL(destroyed()), this, SLOT(threadMessageDeleted()));
    }
    updateItem();
}

ForumThread* ThreadListThreadItem::thread() {
    return _thread;
}

void ThreadListThreadItem::updateItem() {
    ThreadListMessageItem::updateItem();

    if(_thread->hasMoreMessages() && !showMoreItem) { // Need to add show more-button
        showMoreItem = new ThreadListShowMoreItem(this);
    } else if(!_thread->hasMoreMessages() && showMoreItem) { // Need to delete show more-button
        removeChild(showMoreItem);
        delete showMoreItem;
        showMoreItem = 0;
    }
    unreadCountChanged(_thread);
}

void ThreadListThreadItem::unreadCountChanged(ForumThread *thr) {
    if(!msg) return;
    if(!_thread) return;
    Q_ASSERT(thr=_thread);
    int unreads = _thread->unreadCount();
    QString threadSubject = messageSubject;
    QString moreString = QString::null;
    if(thread()->hasMoreMessages()) moreString = "+";
    if (unreads) {
        threadSubject += " (" + QString().number(unreads) + moreString + ")";
    } else if(!moreString.isNull()) {
        threadSubject += " (" + moreString + ")";
    }
    if(!message()) threadSubject += " (no messages, needs update)";
    setText(0, threadSubject);
}

// Commit suicide
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
    setMessage(0);
}

void ThreadListThreadItem::addMessage(ForumMessage *message) {
    Q_ASSERT(message);
    Q_ASSERT(message->thread() == thread());
    Q_ASSERT(message->thread()->group() == _thread->group());

    // DEBUG, check for dupclicates
    for(int i=0;i<childCount();i++) {
        ThreadListMessageItem *item = dynamic_cast<ThreadListMessageItem*> (child(i));
        if(item) {
            if(item->message() == message) {
                Q_ASSERT(item->message() != message);
                Q_ASSERT(item->message()->id() != message->id());
            }
        }
    }

    ThreadListMessageItem *item = 0;
    if(message->ordernum() == 0) { // First message - update thread item!
        Q_ASSERT(!this->message());
        setMessage(message);
        item = this;
        // qDebug() << Q_FUNC_INFO << "setting the thread item";
    } else { // Reply message - create new item
        item = new ThreadListMessageItem(this, message);
    }

    item->updateItem();
    item->updateRead();
}

void ThreadListThreadItem::removeMessage(ForumMessage *message) {
    for(int i=0;i<childCount();i++) {
        ThreadListMessageItem *item = dynamic_cast<ThreadListMessageItem*> (child(i));
        if(item && item->message()==message) {
            if(message->ordernum()==0) {
                setMessage(0);
                updateItem();
            } else {
                takeChild(i);
                delete item;
            }
            return;
        }
    }
}
