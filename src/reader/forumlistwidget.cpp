#include "forumlistwidget.h"

ForumListWidget::ForumListWidget(QWidget *parent, ForumDatabase &f,
		ParserDatabase &p) :
	QToolBox(parent), fdb(f), pdb(p) {
	connect(this, SIGNAL(currentChanged(int)), this,
			SLOT(forumItemSelected(int)));
	connect(&f, SIGNAL(subscriptionFound(ForumSubscription *)), this, SLOT(subscriptionFound(ForumSubscription *)));
	connect(&f, SIGNAL(groupFound(ForumGroup *)), this, SLOT(groupFound(ForumGroup *)));
}

ForumListWidget::~ForumListWidget() {

}
void ForumListWidget::forumItemSelected(int i) {
	emit forumSelected(getSelectedForum());
}

void ForumListWidget::updateForumList() {
        //bool firstRun = (count() == 0);
	//while (count() > 0) {
	//	removeItem(0);
	//}
	//forumIndexes.clear();
	/*
	QList<ForumSubscription*> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		QListWidget *lw = 0;
		if(!forumIndexes.contains(forums[i].parser)) {
			subscriptions[forums[i].parser] = forums[i];
			lw = new QListWidget(this);
			addItem(lw, "later");
			forumIndexes[forums[i].parser] = i;
		} else {
			lw = static_cast<QListWidget*> (widget(forumIndexes[forums[i].parser]));
		}

		lw->clear();

		QList<ForumGroup> groups = fdb.listGroups(forums[i].parser);

		for (int j = 0; j < groups.size(); j++) {
			if (groups[j].subscribed) {
				QListWidgetItem *lwi = new QListWidgetItem(lw);
				int unread = fdb.unreadIn(groups[j]);
				QString title = groups[j].name;
				if (unread > 0)
					title = title + " (" + QString().number(unread) + ")";
				lwi->setText(title);
				lwi->setIcon(QIcon(":/data/folder.png"));
				lw->addItem(lwi);
				forumGroups[lwi] = groups[j];
				if (groups[j].id == currentGroup.id)
					lw->setCurrentItem(lwi);
			}
		}
		disconnect(
				lw,
				SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
				this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
		connect(
				lw,
				SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
				this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));

		// Setup Favicon
		if (forumIcons.contains(forums[i].parser)) {
			forumIcons[forums[i].parser]->update();
		} else {
			QString fiUrl = pdb.getParser(forums[i].parser).forum_url;
			fiUrl = fiUrl.replace(QUrl(fiUrl).path(), "");
			fiUrl = fiUrl + "/favicon.ico";
			Favicon *fi = new Favicon(this, forums[i].parser);
			connect(fi, SIGNAL(iconChanged(int, QIcon)), this,
					SLOT(iconUpdated(int, QIcon)));
			fi->fetchIcon(QUrl(QUrl(fiUrl)), QPixmap(":/data/emblem-web.png"));
			forumIcons[forums[i].parser] = fi;
		}
		widget(i)->setEnabled(true);
		if (!lw->currentItem()) { // Selected group doesn't exist!
			currentGroup = ForumGroup();
			emit groupSelected(currentGroup);
		}

		if (!firstRun && currentGroup.parser == forums[i].parser) {
			setCurrentIndex(i);
		}

	}
	if (firstRun) {
		setCurrentIndex(0);
		emit forumSelected(getSelectedForum());
		emit groupSelected(getSelectedGroup());
	}
	updateReadCounts();
	*/
    emit forumSelected(0);
    emit groupSelected(0);
}

void ForumListWidget::updateReadCounts() {
	/*
	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		int unread = fdb.unreadIn(forums[i]);
		QString title = forums[i].name;
		if (unread > 0)
			title = title + " (" + QString().number(unread) + ")";

		// @todo could be done just for the visible forum
		setItemText(forumIndexes[forums[i].parser], title);
		QListWidget *lw = static_cast<QListWidget*> (widget(
				forumIndexes[forums[i].parser]));
		for (int j = 0; j < lw->count(); j++) {
			QListWidgetItem *it = lw->item(j);
			QFont fnt = it->font();
			int unread = fdb.unreadIn(forumGroups[it]);
			QString title = forumGroups[it].name;
			if (unread > 0)
				title = title + " (" + QString().number(unread) + ")";
			it->setText(title);
			fnt.setBold(unread > 0);
			it->setFont(fnt);
		}
	}
	*/
}

ForumSubscription* ForumListWidget::getSelectedForum() {
	/* @todo tee
	int s = currentIndex();
	QHashIterator<ForumSubscription *, int> hi(forumIndexes);
	while (hi.hasNext()) {
		hi.next();
		if (hi.value() == s)
			return subscriptions[hi.key()];
	}
	*/
	return 0;
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
    forumSubscriptions[forum]->setEnabled(!reloading);
    setItemEnabled(indexOf(forumSubscriptions[forum]), !reloading);
}

void ForumListWidget::groupSelected(QListWidgetItem* item,
		QListWidgetItem *prev) {
	currentGroup = forumGroups[item];
	emit groupSelected(currentGroup);
}

void ForumListWidget::subscriptionFound(ForumSubscription *sub) {
    QListWidget *lw = new QListWidget(this);
    addItem(lw, sub->name());
    forumSubscriptions[sub] = lw;

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
    Q_ASSERT(forumSubscriptions.contains(grp->subscription()));
    QListWidget *lw = forumSubscriptions[grp->subscription()];
    Q_ASSERT(lw);
    QListWidgetItem *lwi = new QListWidgetItem(lw);
    int unread = fdb.unreadIn(grp);
    QString title = grp->name();
    if (unread > 0)
        title = title + " (" + QString().number(unread) + ")";
    lwi->setText(title);
    lwi->setIcon(QIcon(":/data/folder.png"));
    lw->addItem(lwi);
    forumGroups[lwi] = grp;
}
