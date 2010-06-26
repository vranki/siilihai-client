#ifndef THREADLISTMESSAGEITEM_H
#define THREADLISTMESSAGEITEM_H

#include <QTreeWidgetItem>
#include <siilihai/forummessage.h>
#include "messageformatting.h"

class ThreadListMessageItem : public QTreeWidgetItem
{
public:
    ThreadListMessageItem(QTreeWidget *tree);
    ThreadListMessageItem(ThreadListMessageItem *threadItem, ForumMessage *message);
    virtual ForumMessage *message();
    virtual void updateItem();
    void updateRead();
protected:
    QString createMessageSubject();
    ForumMessage *msg;
    QString messageSubject;
};

#endif // THREADLISTMESSAGEITEM_H
