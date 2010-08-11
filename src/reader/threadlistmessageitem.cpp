#include "threadlistmessageitem.h"

ThreadListMessageItem::ThreadListMessageItem(QTreeWidget *tree) : QObject(tree), QTreeWidgetItem(tree) {
    msg = 0;
}

ThreadListMessageItem::ThreadListMessageItem(ThreadListMessageItem *threadItem,
                                             ForumMessage *message) : QTreeWidgetItem(threadItem)
{
    msg = message;
    QString orderString;
    if(message->ordernum() >=0) {
        orderString = QString().number(message->ordernum()).rightJustified(6, '0');
    }
    setText(0, "Call updateItem()");
    setText(3, orderString);
    connect(msg, SIGNAL(changed(ForumMessage*)), this, SLOT(updateItem()));
    connect(msg, SIGNAL(markedRead(ForumMessage*,bool)), this, SLOT(updateRead()));
    connect(msg, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

ForumMessage* ThreadListMessageItem::message() {
    return msg;
}

void ThreadListMessageItem::updateItem() {
    QString orderString;
    // Orderstring is thread's order if first message, or messages if not:
    if(msg->ordernum() == 0) {
        orderString = QString().number(msg->thread()->ordernum()).rightJustified(6, '0');
    } else if(msg->ordernum() > 0) {
        orderString = QString().number(msg->ordernum()).rightJustified(6, '0');
    }
    QString oldOrderString = text(3);

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
}


void ThreadListMessageItem::updateRead() {
    if(!msg) return;
/*
    if(parentItem) {
        updateThreadUnreads(parentItem);
    } else {
        updateThreadUnreads(item);
    }
    */
    QFont myFont = font(0);
    if (msg->read()) {
        myFont.setBold(false);
        setIcon(0, QIcon(":/data/emblem-mail.png"));
    } else {
        myFont.setBold(true);
        setIcon(0, QIcon(":/data/mail-unread.png"));
    }
    setFont(0, myFont);
    /*
    if(parentItem)
        updateMessageRead(parentItem);
        */
}


QString ThreadListMessageItem::createMessageSubject() {
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
