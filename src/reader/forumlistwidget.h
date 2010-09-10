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
        void addSubscription(ForumSubscription *sub); // subscription's parser MUST be set!
public slots:
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
	void forumItemSelected(int i);
        void iconUpdated(ForumSubscription* en, QIcon newIcon);
	void groupFound(ForumGroup *grp);
        void groupChanged(ForumGroup *grp);
        void groupDeleted(QObject*);
        void subscriptionDeleted(QObject*);
        void unsubscribeGroupClicked();
        void markAllReadClicked(bool un=false);
        void markAllUnreadClicked();
        void groupSubscriptionsClicked();
        void updateSubscriptionLabel(ForumSubscription *sub);
        void updateGroupLabel(ForumGroup *grp);
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
	ForumDatabase &fdb;
	ParserDatabase &pdb;
	ForumGroup *currentGroup;
        QMap<ForumSubscription*, Favicon*> forumIcons;
        QMap<QListWidgetItem*, ForumGroup*> forumGroups;
        QMap<ForumSubscription*, QListWidget*> listWidgets;
        // Actions:
        QAction *unsubscribeAction;
        QAction *markReadAction;
        QAction *markUnreadAction;
        QAction *groupSubscriptionsAction;
        QAction *unsubscribeForumAction;
        QAction *forumPropertiesAction;
    };

#endif // FORUMLISTWIDGET_H
