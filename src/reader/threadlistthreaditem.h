#ifndef THREADLISTTHREADITEM_H
#define THREADLISTTHREADITEM_H

#include "siilihai/forummessage.h"
#include "siilihai/forumthread.h"

#include "threadlistmessageitem.h"

class ThreadListThreadItem : public ThreadListMessageItem
{
public:
    ThreadListThreadItem(QTreeWidget *tree, ForumThread *thread);
    void setMessage(ForumMessage *message);
    ForumThread* thread();
    void updateUnreads();
    virtual void updateItem();
private:
    ForumThread *thr;
};

#endif // THREADLISTTHREADITEM_H
