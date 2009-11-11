#include "forumlistwidget.h"

ForumListWidget::ForumListWidget(QWidget *parent, ForumDatabase &f,
		ParserDatabase &p) :
	QToolBox(parent), fdb(f), pdb(p) {

}

ForumListWidget::~ForumListWidget() {

}

void ForumListWidget::updateForumList() {
	while (count() > 0) {
		removeItem(0);
	}
	forumIndexes.clear();
	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		subscriptions[forums[i].parser] = forums[i];

		QListWidget *lw = new QListWidget(this);

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

		addItem(lw, "later");
		forumIndexes[forums[i].parser] = i;

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
	}
	updateReadCounts();
}

void ForumListWidget::updateReadCounts() {
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
}

ForumSubscription ForumListWidget::getSelectedForum() {
	int s = currentIndex();
	QHashIterator<int, int> hi(forumIndexes);
	while (hi.hasNext()) {
		hi.next();
		if (hi.value() == s)
			return subscriptions[hi.key()];
	}
	return ForumSubscription();
}

ForumGroup ForumListWidget::getSelectedGroup() {
	QListWidget *lw = static_cast<QListWidget*> (currentWidget());
	if (lw) {
		QListWidgetItem* it = lw->currentItem();
		if (it) {
			return forumGroups[it];
		}
	}
	return ForumGroup();
}

void ForumListWidget::iconUpdated(int forum, QIcon newIcon) {
	if (forumIndexes.contains(forum))
		setItemIcon(forumIndexes[forum], newIcon);
}

void ForumListWidget::setForumStatus(int forum, bool reloading, float progress) {
	if (!forumIndexes.contains(forum))
		return;
	if (!forumIndexes.contains(forum))
		return;
	Favicon *fi = forumIcons[forum];

	fi->setReloading(reloading, progress);
	widget(forumIndexes[forum])->setEnabled(!reloading);
	setItemEnabled(forumIndexes[forum], !reloading);
}

void ForumListWidget::groupSelected(QListWidgetItem* item,
		QListWidgetItem *prev) {
	emit groupSelected(forumGroups[item]);
}
