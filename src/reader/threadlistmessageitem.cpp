#include "threadlistmessageitem.h"
#include <siilihai/forumthread.h>
#include "threadlistthreaditem.h"

ThreadListMessageItem::ThreadListMessageItem(QTreeWidget *tree) : QObject(tree), QTreeWidgetItem(tree) {
    msg = 0;
}

ThreadListMessageItem::ThreadListMessageItem(ThreadListMessageItem *threadItem, ForumMessage *message) : QTreeWidgetItem(threadItem)
{
    Q_ASSERT(message);
    Q_ASSERT(message->isSane());
    Q_ASSERT(message->thread());
    Q_ASSERT(message->thread()->isSane());
    msg = message;
    QString orderString;
    if(message->ordernum() >=0) {
        orderString = QString().number(message->ordernum()).rightJustified(6, '0');
    }
    lastOrderNum = message->ordernum();
    setText(0, "Call updateItem()");
    setText(3, orderString);
    connect(msg, SIGNAL(changed(ForumMessage*)), this, SLOT(updateItem()));
    connect(msg, SIGNAL(markedRead(ForumMessage*,bool)), this, SLOT(updateRead()));
    connect(msg, SIGNAL(destroyed()), this, SLOT(messageDeleted()));
}


ThreadListMessageItem::~ThreadListMessageItem() {
    disconnect(this);
    msg = 0;
}

ForumMessage* ThreadListMessageItem::message() {
    return msg;
}

void ThreadListMessageItem::updateItem() {
    if(!msg) return; // Possible when this is a ThreadItem and message has not yet been set!
    if(QTreeWidgetItem::parent() && lastOrderNum != 0 && msg->ordernum()==0) { // Acthung! needs to be moved to thread message!
        ThreadListThreadItem *parentItem = static_cast<ThreadListThreadItem*> (QTreeWidgetItem::parent());
        Q_ASSERT(parentItem);
        parentItem->setMessage(message());
        // Commit suicide
        messageDeleted();
        return;
    }
    lastOrderNum = msg->ordernum();
    QString orderString;
    QString oldOrderString = text(3);
    // Orderstring is thread's order if first message, or messages if not:
    if(msg->ordernum() == 0) {
        orderString = QString().number(msg->thread()->ordernum()).rightJustified(6, '0');
    } else if(msg->ordernum() > 0) {
        orderString = QString().number(msg->ordernum()).rightJustified(6, '0');
    }

    messageSubject =  createMessageSubject();
    QString lc = msg->lastchange();
    lc = MessageFormatting::sanitize(lc);
    QString author = msg->author();
    author = MessageFormatting::sanitize(author);
    setText(0, messageSubject);
    setText(1, lc);
    setText(2, author);

    if(oldOrderString != orderString) {
        setText(3, orderString);
    }
    updateRead();
}


void ThreadListMessageItem::updateRead() {
    if(!msg) return;

    QFont myFont = font(0);
    if (msg->isRead()) {
        myFont.setBold(false);
        setIcon(0, QIcon(":/data/emblem-mail.png"));
    } else {
        myFont.setBold(true);
        setIcon(0, QIcon(":/data/mail-unread.png"));
    }
    setFont(0, myFont);
}


QString ThreadListMessageItem::createMessageSubject() {
    QString subj;
    if(msg->name().length() > msg->thread()->name().length()) {
        subj = msg->name();
    } else {
        subj = msg->thread()->name();
        if(msg->ordernum() > 0) subj = "Re: " + subj;
    }
    subj = MessageFormatting::sanitize(subj);
    return subj;
}

void ThreadListMessageItem::messageDeleted() {
    disconnect(this);
    msg = 0;
    QTreeWidgetItem *p = QTreeWidgetItem::parent();
    p->removeChild(this);
    deleteLater();
}
