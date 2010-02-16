#include "forumlistwidget.h"

ForumListWidget::ForumListWidget(QWidget *parent, ForumDatabase &f,
		ParserDatabase &p) :
	QToolBox(parent), fdb(f), pdb(p) {
	connect(this, SIGNAL(currentChanged(int)), this,
			SLOT(forumItemSelected(int)));
        connect(&f, SIGNAL(subscriptionFound(ForumSubscription *)), this, SLOT(subscriptionFound(ForumSubscription *)));
        connect(&f, SIGNAL(groupFound(ForumGroup *)), this, SLOT(groupFound(ForumGroup *)));
        connect(&f, SIGNAL(groupUpdated(ForumGroup *)), this, SLOT(groupUpdated(ForumGroup *)));
        connect(&f, SIGNAL(groupDeleted(ForumGroup *)), this, SLOT(groupDeleted(ForumGroup *)));
        connect(&f, SIGNAL(subscriptionDeleted(ForumSubscription*)), this, SLOT(subscriptionDeleted(ForumSubscription*)));
        connect(&f, SIGNAL(messageUpdated(ForumMessage*)), this, SLOT(messageUpdated(ForumMessage*)));
}

ForumListWidget::~ForumListWidget() {

}
void ForumListWidget::forumItemSelected(int i) {
    qDebug() << Q_FUNC_INFO;
    emit forumSelected(subscriptions[i]);
}

void ForumListWidget::messageUpdated(ForumMessage *msg) {
    groupUpdated(msg->thread()->group());
}

ForumSubscription* ForumListWidget::getSelectedForum() {
        return subscriptions[currentIndex()];
}

ForumGroup* ForumListWidget::getSelectedGroup() {
	QListWidget *lw = static_cast<QListWidget*> (currentWidget());
	if (lw) {
		QListWidgetItem* it = lw->currentItem();
		if (it) {
			return forumGroups[it];
		}
	}
	return 0;
}

void ForumListWidget::iconUpdated(ForumSubscription *forum, QIcon newIcon) {
    Q_ASSERT(forumSubscriptions.contains(forum));
    setItemIcon(indexOf(forumSubscriptions[forum]), newIcon);
}

void ForumListWidget::setForumStatus(ForumSubscription* forum, bool reloading, float progress) {
    Q_ASSERT(forumSubscriptions.contains(forum));
    Favicon *fi = forumIcons[forum];

    fi->setReloading(reloading, progress);
    //forumSubscriptions[forum]->setEnabled(!reloading);
    //setItemEnabled(indexOf(forumSubscriptions[forum]), !reloading);
}

void ForumListWidget::groupSelected(QListWidgetItem* item,
		QListWidgetItem *prev) {
	currentGroup = forumGroups[item];
	emit groupSelected(currentGroup);
}

void ForumListWidget::subscriptionFound(ForumSubscription *sub) {
    qDebug() << Q_FUNC_INFO << sub << currentIndex();
    Q_ASSERT(sub);
    QListWidget *lw = new QListWidget(this);
    subscriptions[count()] = sub;
    forumSubscriptions[sub] = lw;
    addItem(lw, sub->name());
    connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
            this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));

    QString fiUrl = pdb.getParser(sub->parser()).forum_url;
    fiUrl = fiUrl.replace(QUrl(fiUrl).path(), "");
    fiUrl = fiUrl + "/favicon.ico";
    Favicon *fi = new Favicon(this, sub);
    connect(fi, SIGNAL(iconChanged(ForumSubscription*, QIcon)), this,
            SLOT(iconUpdated(ForumSubscription*, QIcon)));
    fi->fetchIcon(QUrl(QUrl(fiUrl)), QPixmap(":/data/emblem-web.png"));
    forumIcons[sub] = fi;
}

void ForumListWidget::groupFound(ForumGroup *grp) {
    Q_ASSERT(grp);
    if(!grp->subscribed()) return;
    Q_ASSERT(forumSubscriptions.contains(grp->subscription()));
    QListWidget *lw = forumSubscriptions[grp->subscription()];
    Q_ASSERT(lw);
    QListWidgetItem *lwi = new QListWidgetItem(lw);
    lwi->setIcon(QIcon(":/data/folder.png"));
    lw->addItem(lwi);
    forumGroups[lwi] = grp;
    groupUpdated(grp);
}


void ForumListWidget::groupUpdated(ForumGroup *grp) {
    Q_ASSERT(grp);
    QListWidget *lw = forumSubscriptions[grp->subscription()];
    Q_ASSERT(lw);
    QListWidgetItem *groupItem = groupItem(grp);
    if(groupItem && !grp->subscribed()) {
        qDebug() << Q_FUNC_INFO << " deleting removed group from UI";
        lw->takeItem(lw->row(groupItem));
        forumGroups.remove(groupItem);
        delete groupItem;
        if(currentGroup == grp) {
            currentGroup = 0;
            emit groupSelected(currentGroup);
        }
    } else if(groupItem && grp->subscribed()) {
        int unread = fdb.unreadIn(grp);
        QString title = grp->name();
        if (unread > 0)
            title = title + " (" + QString().number(unread) + ")";
        qDebug() << Q_FUNC_INFO << " Updating group title to " << title;
        groupItem->setText(title);
    } else if(!groupItem && grp->subscribed()) {
        qDebug() << Q_FUNC_INFO << " adding subscribed group to UI";
        groupFound(grp);
    }
}

void ForumListWidget::groupDeleted(ForumGroup *grp) {
    if(!grp->subscribed()) return;
    QListWidgetItem *item = groupItem(grp);
    Q_ASSERT(item);
    // @todo jatka
}

void ForumListWidget::subscriptionDeleted(ForumSubscription *sub) {

}

QListWidgetItem * ForumListWidget::groupItem(ForumGroup *grp) {
    foreach(QListWidgetItem *lwi, forumGroups.keys()) {
        if(forumGroups[lwi] == grp) {
            return lwi;
        }
    }
    return 0;
}
