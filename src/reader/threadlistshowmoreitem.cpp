#include "threadlistshowmoreitem.h"

ThreadListShowMoreItem::ThreadListShowMoreItem(QTreeWidgetItem *threadItem) : QTreeWidgetItem(threadItem)
{
    Q_ASSERT(threadItem);
    setText(0, "Show More messages");
    setText(3, "999999"); // Always the last one
}
