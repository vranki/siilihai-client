#ifndef THREADLISTMESSAGEITEM_H
#define THREADLISTMESSAGEITEM_H

#include <QTreeWidgetItem>
#include <siilihai/forumdata/forummessage.h>
#include <QObject>

class ThreadListMessageItem :  public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    enum IconImage { II_UNDEFINED, II_UNREAD, II_READ };


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
    virtual void setIconImage(IconImage newIcon);
    ForumMessage *msg;
    QString messageSubject;
    int lastOrderNum;
    IconImage currentIconImage;
};

#endif // THREADLISTMESSAGEITEM_H
