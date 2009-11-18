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
	ForumGroup currentGroup;
	QHash<int, ForumSubscription> subscriptions;
	QHash<int, int> forumIndexes;
	QHash<int, Favicon*> forumIcons;
	QHash<QListWidgetItem*, ForumGroup> forumGroups;

};

#endif // FORUMLISTWIDGET_H
