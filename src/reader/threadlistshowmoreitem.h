#ifndef THREADLISTSHOWMOREITEM_H
#define THREADLISTSHOWMOREITEM_H

#include <QTreeWidgetItem>
#include <siilihai/forumthread.h>
#include "threadlistthreaditem.h"

class ThreadListShowMoreItem : public QTreeWidgetItem
{
public:
    ThreadListShowMoreItem(ThreadListThreadItem *threadItem);
    ThreadListThreadItem *threadItem();
private:
    ThreadListThreadItem *thr;
};

#endif // THREADLISTSHOWMOREITEM_H
