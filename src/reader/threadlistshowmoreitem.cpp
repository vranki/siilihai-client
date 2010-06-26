#include "threadlistshowmoreitem.h"

ThreadListShowMoreItem::ThreadListShowMoreItem(ThreadListThreadItem *threadItem) : QTreeWidgetItem(threadItem)
{
    Q_ASSERT(threadItem);
    Q_ASSERT(threadItem);
    Q_ASSERT(threadItem->thread()->isSane());
    thr = threadItem;
    setText(0, "Show More messages");
    setText(3, "999999"); // Always the last one
}

ThreadListThreadItem *ThreadListShowMoreItem::threadItem() {
    return thr;
}
