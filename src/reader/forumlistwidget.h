#ifndef FORUMLISTWIDGET_H
#define FORUMLISTWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QToolBox>
#include <QHash>
#include <QListWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>

#include <siilihai/parserdatabase.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/forumsubscription.h>
#include <siilihai/forumgroup.h>

#include "favicon.h"

class ForumListWidget: public QToolBox {
Q_OBJECT

public:
	ForumListWidget(QWidget *parent, ForumDatabase &f, ParserDatabase &p);
	~ForumListWidget();
	void updateReadCounts();
	void updateForumList();
	ForumSubscription *getSelectedForum();
	ForumGroup *getSelectedGroup();
	void setForumStatus(ForumSubscription* forum, bool reloading, float progress);
public slots:
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
	void forumItemSelected(int i);
	void iconUpdated(ForumSubscription *sub, QIcon newIcon);
	void subscriptionFound(ForumSubscription *sub);
	void groupFound(ForumGroup *grp);
signals:
	void groupSelected(ForumGroup *grp);
	void forumSelected(ForumSubscription *sub);

private:
	ForumDatabase &fdb;
	ParserDatabase &pdb;
	ForumGroup *currentGroup;
	QHash<ForumSubscription*, int> subscriptions; // id, sub
	QHash<ForumSubscription*, int> forumIndexes; // id, idx
	QHash<ForumSubscription*, Favicon*> forumIcons;
	QHash<QListWidgetItem*, ForumGroup*> forumGroups;
};

#endif // FORUMLISTWIDGET_H
