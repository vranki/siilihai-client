#include "forumlistwidget.h"

ForumListWidget::ForumListWidget(QWidget *parent, ForumDatabase &f,
                                 ParserDatabase &p) :
QToolBox(parent), fdb(f), pdb(p) {
    connect(this, SIGNAL(currentChanged(int)), this,
            SLOT(forumItemSelected(int)));
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
   // qDebug() << Q_FUNC_INFO << " selected forum index " << i;
    ForumSubscription *sub = 0;
    if(i != -1) {
        QHashIterator<ParserEngine*, QListWidget*> it(parserEngines);
        while (it.hasNext()) {
            it.next();
            if(indexOf(it.value()) == i) {
                sub = it.key()->subscription();
            }
        }
        Q_ASSERT(sub);
    }
/*
 @todo why this?
    foreach(ForumGroup *grp, fdb.listGroups(sub))
        groupChanged(grp);
*/
    emit forumSelected(sub);
}
/*
void ForumListWidget::messageUpdated(ForumMessage *msg) {
    groupUpdated(msg->thread()->group());
}
*/
ForumSubscription* ForumListWidget::getSelectedForum() {
    ForumSubscription *sub = 0;
    QListWidget *curWidget = static_cast<QListWidget*> (currentWidget());
    if(curWidget) {
        sub = parserEngines.key(curWidget)->subscription(); // Can be NULL when quitting
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

void ForumListWidget::iconUpdated(ParserEngine* en, QIcon newIcon) {
//    qDebug() << Q_FUNC_INFO << " setting icon for engine " << en->subscription()->toString()
 //           << " pos " << indexOf(parserEngines[en]);
    setItemIcon(indexOf(parserEngines[en]), newIcon);
}

ParserEngine* ForumListWidget::engineOf(ForumSubscription *sub) {
    foreach(ParserEngine *en, parserEngines.keys()) {
        if(en->subscription() == sub) return en;
    }
    return 0;
}

void ForumListWidget::groupSelected(QListWidgetItem* item,
                                    QListWidgetItem *prev) {
   // qDebug() << Q_FUNC_INFO << " selected item " << item << ", prev " << prev;
    currentGroup = forumGroups[item];
    emit groupSelected(currentGroup);
}

void ForumListWidget::addParserEngine(ParserEngine *pe) {
    qDebug() << Q_FUNC_INFO;
    QListWidget *lw = new QListWidget(this);
    parserEngines[pe] = lw;
    addItem(lw, pe->subscription()->alias());
    connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)),
            this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
    connect(pe->subscription(), SIGNAL(unreadCountChanged(ForumSubscription*)), this, SLOT(updateSubscriptionLabel(ForumSubscription*)));
    connect(pe->subscription(), SIGNAL(destroyed(QObject*)), this, SLOT(subscriptionDeleted(QObject*)));

    QString fiUrl = pdb.getParser(pe->subscription()->parser()).forum_url;
    fiUrl = fiUrl.replace(QUrl(fiUrl).path(), "");
    fiUrl = fiUrl + "/favicon.ico";
    Favicon *fi = new Favicon(this, pe);
    connect(fi, SIGNAL(iconChanged(ParserEngine*, QIcon)), this,
            SLOT(iconUpdated(ParserEngine*, QIcon)));
    fi->fetchIcon(QUrl(QUrl(fiUrl)), QPixmap(":/data/emblem-web.png"));
    forumIcons[pe] = fi;
}

void ForumListWidget::groupFound(ForumGroup *grp) {
    // qDebug() << Q_FUNC_INFO << grp->toString();
    Q_ASSERT(grp);
    connect(grp, SIGNAL(changed(ForumGroup*)), this, SLOT(groupChanged(ForumGroup*)));
    connect(grp, SIGNAL(unreadCountChanged(ForumGroup*)), this, SLOT(updateGroupLabel(ForumGroup*)));
    connect(grp, SIGNAL(destroyed(QObject*)), this, SLOT(groupDeleted(QObject*)));

    if(!grp->subscribed()) return;
    QListWidget *lw = parserEngines[engineOf(grp->subscription())];
    Q_ASSERT(lw);
    QListWidgetItem *lwi = new QListWidgetItem(lw);
    lwi->setIcon(QIcon(":/data/folder.png"));
    lw->addItem(lwi);
    forumGroups[lwi] = grp;
    groupChanged(grp); // Set title etc.
}


void ForumListWidget::groupChanged(ForumGroup *grp) {
    Q_ASSERT(grp);
    if(!engineOf(grp->subscription())) return; // May happen while quitting
    QListWidget *lw = parserEngines.value(engineOf(grp->subscription()));
    Q_ASSERT(lw);
    QListWidgetItem *gItem = groupItem(grp);
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
    QListWidget *lw = parserEngines.value(engineOf(sub));
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
    if(!engineOf(grp->subscription())) return; // May happen while quittinq
    QListWidget *lw = parserEngines.value(engineOf(grp->subscription()));
    Q_ASSERT(lw);
    QListWidgetItem *gItem = groupItem(grp);
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
    QListWidgetItem *item = groupItem(grp);
    Q_ASSERT(item);
    QListWidget *lw = parserEngines[engineOf(grp->subscription())];
    Q_ASSERT(lw);
    lw->removeItemWidget(item);
    forumGroups.remove(item);
    delete item;
}

void ForumListWidget::subscriptionDeleted(QObject *s) {
    ForumSubscription *sub = static_cast<ForumSubscription*>(s);
    if(currentGroup && currentGroup->subscription()==sub) {
        currentGroup = 0;
        emit forumSelected(0);
    }
    QListWidget *lw = parserEngines[engineOf(sub)];
    Q_ASSERT(lw);
    removeItem(indexOf(lw));
    parserEngines.remove(engineOf(sub));
    lw->deleteLater();
}

QListWidgetItem * ForumListWidget::groupItem(ForumGroup *grp) {
    foreach(QListWidgetItem *lwi, forumGroups.keys()) {
        if(forumGroups[lwi] == grp) {
            return lwi;
        }
    }
    return 0;
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
            foreach(ForumMessage *msg, thread->messages()) {
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
