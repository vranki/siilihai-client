#ifndef FORUMLISTWIDGET_H
#define FORUMLISTWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QToolBox>
#include <QHash>
#include <QListWidgetItem>

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>
#include <forumgroup.h>

#include "favicon.h"

class ForumListWidget: public QToolBox {
Q_OBJECT

public:
	ForumListWidget(QWidget *parent, ForumDatabase &f, ParserDatabase &p);
	~ForumListWidget();
	void updateReadCounts();
	void updateForumList();
	ForumSubscription getSelectedForum();
	ForumGroup getSelectedGroup();
	void setForumStatus(int forum, bool reloading, float progress);
public slots:
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
	void iconUpdated(int forum, QIcon newIcon);
signals:
	void groupSelected(ForumGroup grp);

private:
	ForumDatabase &fdb;
	ParserDatabase &pdb;
	QHash<int, ForumSubscription> subscriptions;
	QHash<int, int> forumIndexes;
	QHash<int, Favicon*> forumIcons;
	QHash<QListWidgetItem*, ForumGroup> forumGroups;

};

#endif // FORUMLISTWIDGET_H
