#ifndef THREADLISTTHREADITEM_H
#define THREADLISTTHREADITEM_H

#include "siilihai/forummessage.h"
#include "siilihai/forumthread.h"
#include "threadlistshowmoreitem.h"
#include "threadlistmessageitem.h"
#include <QObject>

class ThreadListThreadItem : public ThreadListMessageItem
{
    Q_OBJECT

public:
    ThreadListThreadItem(QTreeWidget *tree, ForumThread *thread);
    void setMessage(ForumMessage *message);
    ForumThread* thread();
public slots:
    void updateUnreads();
    virtual void updateItem();
    void threadDeleted();
    void threadMessageDeleted(); // Called when the message is deleted
private:
    ForumThread *thr;
    ThreadListShowMoreItem* showMoreItem;
    QTreeWidget *treeWidget;
};

#endif // THREADLISTTHREADITEM_H
