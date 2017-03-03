#include "threadlistmessageitem.h"
#include <siilihai/updateengine.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/messageformatting.h>
#include "threadlistthreaditem.h"

ThreadListMessageItem::ThreadListMessageItem(QTreeWidget *tree) : QObject(tree), QTreeWidgetItem(tree) {
    msg = 0;
    currentIconImage = II_UNDEFINED;
}

ThreadListMessageItem::ThreadListMessageItem(ThreadListMessageItem *threadItem, ForumMessage *message) : QTreeWidgetItem(threadItem)
{
    Q_ASSERT(message);
    Q_ASSERT(message->isSane());
    Q_ASSERT(message->thread());
    Q_ASSERT(message->thread()->isSane());
    currentIconImage = II_UNDEFINED;
    msg = message;
    QString orderString;
    if(message->ordernum() >=0) {
        orderString = QString().number(message->ordernum()).rightJustified(6, '0');
    }
    lastOrderNum = message->ordernum();
    setText(0, "Call updateItem()");
    setText(3, orderString);
    connect(msg, SIGNAL(changed()), this, SLOT(updateItem()));
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

    messageSubject = msg->displayName();
    QString lc = msg->lastchange();
    lc = MessageFormatting::sanitize(lc);

    // UE can be null in some situation, avoid crashing
    if(msg->thread()->group()->subscription()->updateEngine())
        lc = msg->thread()->group()->subscription()->updateEngine()->convertDate(lc);

    QString author = msg->author();
    author = MessageFormatting::sanitize(author);
    setText(0, messageSubject);
    setText(1, lc);
    setText(2, author);

    if(oldOrderString != orderString) {
        setText(3, orderString);
    }
    updateRead();
    emit requestSorting();
}


void ThreadListMessageItem::updateRead() {
    if(!msg) return;

    QFont myFont = font(0);
    myFont.setBold(!msg->isRead());
    setIconImage(msg->isRead() ? II_READ : II_UNREAD);
    setFont(0, myFont);
}

void ThreadListMessageItem::messageDeleted() {
    disconnect(this);
    msg = 0;
    QTreeWidgetItem *p = QTreeWidgetItem::parent();
    p->removeChild(this);
    deleteLater();
}

void ThreadListMessageItem::setIconImage(IconImage newIcon) {
    if(newIcon == currentIconImage) return;
    currentIconImage = newIcon;

    if (newIcon == II_READ) {
        setIcon(0, QIcon(":/data/emblem-mail.png"));
    } else if(newIcon == II_UNREAD) {
        setIcon(0, QIcon(":/data/mail-unread.png"));
    }
}
