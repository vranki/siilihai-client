#include "threadlistthreaditem.h"

ThreadListThreadItem::ThreadListThreadItem(QTreeWidget *tree, ForumThread *thread) : ThreadListMessageItem(tree)
{
    Q_ASSERT(thread);
    thr = thread;
    Q_ASSERT(thread->isSane());
    QString threadSubject = thread->name();//messageSubject(thread->name());
    QString lc = thread->lastchange();
    QString author = "";
    QString orderString;
    if(thread->ordernum() >=0) {
        orderString = QString().number(thread->ordernum()).rightJustified(6, '0');
    }
    setText(0, threadSubject);
    setText(1, MessageFormatting::sanitize(lc));
    setText(2, MessageFormatting::sanitize(author));
    setText(3, orderString);
}

void ThreadListThreadItem::setMessage(ForumMessage *message) {
    msg = message;
    updateItem();
}

ForumThread* ThreadListThreadItem::thread() {
    return thr;
}

void ThreadListThreadItem::updateItem() {
    ThreadListMessageItem::updateItem();
    updateUnreads();
}

void ThreadListThreadItem::updateUnreads() {
    if(!msg) return;
    int unreads = 0;
    if(!msg->read())
        unreads++; // Also count first message

    for(int i=0;i<childCount();i++) {
        ThreadListMessageItem* messageItem = dynamic_cast<ThreadListMessageItem*>(child(i));
        if(messageItem) {
            if(!messageItem->message()->read())
                unreads++;
        }
    }

    QString threadSubject = messageSubject;
    if (unreads) {
        threadSubject += " (" + QString().number(unreads) + ")";
    }
    setText(0, threadSubject);
}
