#include "forumlistwidget.h"

ForumListWidget::ForumListWidget(QWidget *parent, ForumDatabase &f, ParserDatabase &p) :
    QToolBox(parent), fdb(f), pdb(p) {
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(forumItemSelected(int)));
    connect(&fdb, SIGNAL(groupFound(ForumGroup *)), this, SLOT(groupFound(ForumGroup *)));

    markReadAction = new QAction("Mark all messages read", this);
    markReadAction->setToolTip("Mark all messages in selected group as read");
    connect(markReadAction, SIGNAL(triggered()), this, SLOT(markAllReadClicked()));
    markUnreadAction = new QAction("Mark all messages unread", this);
    markUnreadAction->setToolTip("Mark all messages in selected group as unread");
    connect(markUnreadAction, SIGNAL(triggered()), this, SLOT(markAllUnreadClicked()));
    unsubscribeAction = new QAction("Unsubscribe group..", this);
    unsubscribeAction->setToolTip("Unsubscribe from the selected group");
    connect(unsubscribeAction, SIGNAL(triggered()), this, SLOT(unsubscribeGroupClicked()));
    groupSubscriptionsAction = new QAction("Group subscriptions..", this);
    groupSubscriptionsAction->setToolTip("Subscribe and unsubscribe to groups in this forum");
    connect(groupSubscriptionsAction, SIGNAL(triggered()), this, SLOT(groupSubscriptionsClicked()));
    unsubscribeForumAction = new QAction("Unsubscribe forum..",this);
    unsubscribeForumAction->setToolTip("Unsubscribe from the forum");
    connect(unsubscribeForumAction, SIGNAL(triggered()), this, SIGNAL(unsubscribeForum()));
    forumPropertiesAction = new QAction("Forum properties..", this);
    forumPropertiesAction->setToolTip("View and edit detailed information about the forum");
    connect(forumPropertiesAction, SIGNAL(triggered()), this, SIGNAL(forumProperties()));
}

ForumListWidget::~ForumListWidget() {

}

void ForumListWidget::forumItemSelected(int i) {
    QListWidget *curWidget = static_cast<QListWidget*> (currentWidget());
    ForumSubscription *sub = 0;
    if(i >= 0) {
        sub = listWidgets.key(static_cast<QListWidget*> (widget(i)));
        if(sub->groups().isEmpty()) {
            emit groupSelected(0);
        } else { // Select the first group
            emit groupSelected(*sub->groups().begin());
            static_cast<QListWidget*> (widget(i))->setCurrentRow(0);
        }/*
        if(listWidgets.value(sub)->currentItem()) {
            groupSelected(listWidgets.value(sub)->currentItem(), 0);
        }*/
    }
    emit forumSelected(sub);
}

ForumSubscription* ForumListWidget::getSelectedForum() {
    ForumSubscription *sub = 0;
    QListWidget *curWidget = static_cast<QListWidget*> (currentWidget());
    if(curWidget) {
        sub = listWidgets.key(curWidget); // Can be NULL when quitting(?)
    }
    return sub;
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

void ForumListWidget::iconUpdated(ForumSubscription* en, QIcon newIcon) {
//    qDebug() << Q_FUNC_INFO << " setting icon for engine " << en->subscription()->toString()
 //           << " pos " << indexOf(parserEngines[en]);
    setItemIcon(indexOf(listWidgets[en]), newIcon);
}

void ForumListWidget::groupSelected(QListWidgetItem* item, QListWidgetItem *prev) {
   // qDebug() << Q_FUNC_INFO << " selected item " << item << ", prev " << prev;
    currentGroup = forumGroups[item];
    emit groupSelected(currentGroup);
}

void ForumListWidget::addSubscription(ForumSubscription *sub) {
    QListWidget *lw = new QListWidget(this);
    listWidgets[sub] = lw;
    addItem(lw, sub->alias());
    connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)), this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
    connect(sub, SIGNAL(unreadCountChanged(ForumSubscription*)), this, SLOT(updateSubscriptionLabel(ForumSubscription*)));
    connect(sub, SIGNAL(destroyed(QObject*)), this, SLOT(subscriptionDeleted(QObject*)));

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
    // qDebug() << Q_FUNC_INFO << grp->toString();
    Q_ASSERT(grp);
    connect(grp, SIGNAL(changed(ForumGroup*)), this, SLOT(groupChanged(ForumGroup*)));
    connect(grp, SIGNAL(unreadCountChanged(ForumGroup*)), this, SLOT(updateGroupLabel(ForumGroup*)));
    connect(grp, SIGNAL(destroyed(QObject*)), this, SLOT(groupDeleted(QObject*)));

    if(!grp->subscribed()) return;
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *lwi = new QListWidgetItem(lw);
    lwi->setIcon(QIcon(":/data/folder.png"));
    lw->addItem(lwi);
    forumGroups[lwi] = grp;
    groupChanged(grp); // Set title etc.
}


void ForumListWidget::groupChanged(ForumGroup *grp) {
    Q_ASSERT(grp);
    qDebug() << Q_FUNC_INFO << grp->toString();
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *gItem = forumGroups.key(grp);
    if(gItem && !grp->subscribed()) {
        // delete unsubscribed group from UI
        lw->takeItem(lw->row(gItem));
        forumGroups.remove(gItem);
        delete gItem;
        if(currentGroup == grp) {
            currentGroup = 0;
            emit groupSelected(currentGroup);
        }
    } else if(gItem && grp->subscribed()) {
        updateGroupLabel(grp);
    } else if(!gItem && grp->subscribed()) {
        // Add subscribed group to UI
        groupFound(grp);
    }
}

void ForumListWidget::updateSubscriptionLabel(ForumSubscription* sub) {
    QListWidget *lw = listWidgets.value(sub);
    // Update subscription message count
    QString title = sub->alias();

    if(sub->unreadCount() > 0)
        title = QString("%1 (%2)").arg(title).arg(sub->unreadCount());
    setItemText(indexOf(lw), title);
}

void ForumListWidget::updateGroupLabel(ForumGroup* grp) {
    // qDebug() << Q_FUNC_INFO;
    Q_ASSERT(grp);
    if(!grp->subscribed()) return;
    if(!grp->subscription()) return; // May happen while quittinq
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *gItem = forumGroups.key(grp);
    QString title = grp->name();
    if (grp->unreadCount() > 0)
        title = title + " (" + QString().number(grp->unreadCount()) + ")";
    gItem->setText(title);
}

void ForumListWidget::groupDeleted(QObject* g) {
    ForumGroup *grp = static_cast<ForumGroup*> (g);
    if(!grp->subscribed()) return;
    if(currentGroup==grp) {
        currentGroup = 0;
        emit groupSelected(0);
    }
    QListWidgetItem *item = forumGroups.key(grp);
    if(!item) {
        qDebug() << Q_FUNC_INFO << " item for group " << grp << "doesn't exist, why??";
        return;
    }
    Q_ASSERT(item);
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    lw->removeItemWidget(item);
    forumGroups.remove(item);
    delete item;
}

void ForumListWidget::subscriptionDeleted(QObject *s) {
    ForumSubscription *sub = static_cast<ForumSubscription*>(s);
    if(currentGroup && currentGroup->subscription()==sub) {
        currentGroup = 0;
        emit groupSelected(0);
        emit forumSelected(0);
    }
    QListWidget *lw = listWidgets.value(sub);
    Q_ASSERT(lw);
    removeItem(indexOf(lw));
    Q_ASSERT(listWidgets.remove(sub));
    lw->deleteLater();
}

void ForumListWidget::contextMenuEvent(QContextMenuEvent *event) {
    QListWidget *lw = qobject_cast<QListWidget*>(currentWidget());

    // @todo could use lw->itemAt(event->pos()) to detect when
    // not clicking on item.
    if(lw) {
        QMenu menu(this);
        if(lw->itemAt(lw->mapFromGlobal(event->globalPos()))) {
            menu.addAction(markReadAction);
            menu.addAction(markUnreadAction);
            menu.addSeparator();
            menu.addAction(unsubscribeAction);
        }
        menu.addAction(groupSubscriptionsAction);
        menu.addSeparator();
        menu.addAction(forumPropertiesAction);
        menu.addAction(unsubscribeForumAction);
        menu.exec(event->globalPos());
    }
}

void ForumListWidget::unsubscribeGroupClicked() {
    if(currentGroup) {
        QMessageBox msgBox;
        msgBox.setText("Really unsubscribe from group?");
        msgBox.setInformativeText(currentGroup->name());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec()==QMessageBox::Yes)
            emit unsubscribeGroup(currentGroup);
    }
}

void ForumListWidget::markAllReadClicked(bool un) {
    if(currentGroup) {
        foreach(ForumThread *thread, currentGroup->threads()) {
            foreach(ForumMessage *msg, thread->values()) {
                msg->setRead(!un);
            }
        }
    }
}

void ForumListWidget::markAllUnreadClicked() {
    markAllReadClicked(true);
}

void ForumListWidget::groupSubscriptionsClicked() {
    if(getSelectedForum()) emit groupSubscriptions(getSelectedForum());
}
