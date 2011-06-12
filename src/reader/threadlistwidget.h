#ifndef THREADLISTWIDGET_H_
#define THREADLISTWIDGET_H_

#include <QTreeWidget>
#include <QList>
#include <QPair>
#include <QMenu>
#include <QContextMenuEvent>
#include <QObject>
#include <QMessageBox>

#include <siilihai/forumgroup.h>
#include <siilihai/forumthread.h>
#include <siilihai/forummessage.h>
#include <siilihai/forumdatabase.h>

#include "messageformatting.h"
#include "threadlistmessageitem.h"
#include "threadlistthreaditem.h"
#include "threadlistshowmoreitem.h"

class ThreadListWidget : public QTreeWidget {
	Q_OBJECT

public:
        ThreadListWidget(QWidget *parent, ForumDatabase &f);
	virtual ~ThreadListWidget();

public slots:
	void groupSelected(ForumGroup *fg);
        void messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev);
        void groupChanged(ForumGroup *grp);
        void groupDeleted(QObject*);
        void markReadClicked(bool read=true);
        void markUnreadClicked();
        void threadPropertiesClicked();
        void viewInBrowserClicked();
        void forceUpdateThreadClicked();
        void selectNextUnread();
private slots:
        void threadFound(ForumThread *thread);
        void removeThread(ForumThread *thread);

        void messageFound(ForumMessage *msg);
        void removeMessage(ForumMessage *msg);

signals:
	void messageSelected(ForumMessage *msg);
        void moreMessagesRequested(ForumThread *thread);
        void viewInBrowser();
        void threadProperties(ForumThread *thread);
        void updateThread(ForumThread *thread, bool force);
protected:
        void contextMenuEvent(QContextMenuEvent * event);

private:
        QString messageSubject(ForumMessage *msg);
        void updateList();
        void clearList();
        void addThread(ForumThread *thread);
        void addMessage(ForumMessage *msg);
        void updateThreadUnreads(ThreadListThreadItem* threadItem);
        void addShowMoreButton(ForumThread *thread);

        // Helpers to quickly find correct items
        QHash<ThreadListThreadItem*, ForumThread*> forumThreads;
        QHash<ThreadListMessageItem*, ForumMessage*> forumMessages;
        ForumGroup *currentGroup;
        ForumDatabase &fdb;
        // Actions:
        QAction *markReadAction;
        QAction *markUnreadAction;
        QAction *threadPropertiesAction;
        QAction *viewInBrowserAction;
        QAction *forceUpdateThreadAction;
        bool disableSortAndResize; // Don't sort or resize while doing long add
};

#endif /* THREADLISTWIDGET_H_ */
