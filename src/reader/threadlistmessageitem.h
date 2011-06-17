#ifndef THREADLISTMESSAGEITEM_H
#define THREADLISTMESSAGEITEM_H

#include <QTreeWidgetItem>
#include <siilihai/forummessage.h>
#include "messageformatting.h"
#include <QObject>

class ThreadListMessageItem :  public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    ThreadListMessageItem(QTreeWidget *tree);
    ThreadListMessageItem(ThreadListMessageItem *threadItem, ForumMessage *message);
    ~ThreadListMessageItem();
    virtual ForumMessage *message();
signals:
    void requestSorting();
public slots:
    void updateRead();
    virtual void updateItem();
    void messageDeleted();

protected:
    QString createMessageSubject();
    ForumMessage *msg;
    QString messageSubject;
    int lastOrderNum;
};

#endif // THREADLISTMESSAGEITEM_H
