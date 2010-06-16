#ifndef FORUMLISTWIDGET_H
#define FORUMLISTWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QToolBox>
#include <QHash>
#include <QListWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

#include <siilihai/parserdatabase.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/forumsubscription.h>
#include <siilihai/parserengine.h>
#include <siilihai/forumgroup.h>

#include "favicon.h"

class ForumListWidget: public QToolBox {
Q_OBJECT

public:
	ForumListWidget(QWidget *parent, ForumDatabase &f, ParserDatabase &p);
	~ForumListWidget();
        void updateReadCounts(ForumGroup *grp);
	ForumSubscription *getSelectedForum();
	ForumGroup *getSelectedGroup();
        void addParserEngine(ParserEngine *pe);
public slots:
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
	void forumItemSelected(int i);
        void iconUpdated(ParserEngine* en, QIcon newIcon);
	void groupFound(ForumGroup *grp);
        void groupUpdated(ForumGroup *grp);
        void groupDeleted(ForumGroup *grp);
        void subscriptionDeleted(ForumSubscription *sub);
        void messageUpdated(ForumMessage *grp);
        void unsubscribeGroupClicked();
        void markAllReadClicked(bool un=false);
        void markAllUnreadClicked();
        void groupSubscriptionsClicked();
signals:
	void groupSelected(ForumGroup *grp);
        void forumSelected(ForumSubscription *sub);
        void unsubscribeGroup(ForumGroup *grp);
        void groupSubscriptions(ForumSubscription *sub);
        void unsubscribeForum();
        void forumProperties();
protected:
        void contextMenuEvent(QContextMenuEvent * event);
private:
        QListWidgetItem *groupItem(ForumGroup *grp);
        ParserEngine* engineOf(ForumSubscription *sub);
	ForumDatabase &fdb;
	ParserDatabase &pdb;
	ForumGroup *currentGroup;
        QHash<ParserEngine*, Favicon*> forumIcons;
	QHash<QListWidgetItem*, ForumGroup*> forumGroups;
        QHash<ParserEngine*, QListWidget*> parserEngines;
        // Actions:
        QAction *unsubscribeAction;
        QAction *markReadAction;
        QAction *markUnreadAction;
        QAction *groupSubscriptionsAction;
        QAction *unsubscribeForumAction;
        QAction *forumPropertiesAction;
    };

#endif // FORUMLISTWIDGET_H
