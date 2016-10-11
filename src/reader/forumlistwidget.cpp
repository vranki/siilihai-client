#include "forumlistwidget.h"
#include "favicon.h"
#include <siilihai/messageformatting.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/parser/parserdatabase.h>
#include <siilihai/forumdatabase/forumdatabase.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/forumgroup.h>


ForumListWidget::ForumListWidget(QWidget *parent) : QToolBox(parent), currentGroup(0) {
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(forumItemSelected(int)));

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
    ForumSubscription *sub = 0;
    ForumGroup *g = 0;
    if(i >= 0) {
        sub = listWidgets.key(dynamic_cast<QListWidget*> (widget(i)));
        for(ForumGroup *grp : sub->values()) {
            if(!g && grp->isSubscribed())
                g = grp;
        }
        dynamic_cast<QListWidget*> (widget(i))->setCurrentRow(0);
    }
    emit forumSelected(sub);
    emit groupSelected(g);
    if(!sub->errorList().isEmpty()) emit displaySubscriptionErrors(sub);
}

ForumSubscription* ForumListWidget::getSelectedForum() {
    ForumSubscription *sub = 0;
    QListWidget *curWidget = dynamic_cast<QListWidget*> (currentWidget());
    if(curWidget) {
        sub = listWidgets.key(curWidget); // Can be NULL when quitting(?)
    }
    return sub;
}

ForumGroup* ForumListWidget::getSelectedGroup() {
    QListWidget *lw = dynamic_cast<QListWidget*> (currentWidget());
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
    Q_UNUSED(prev);
    // qDebug() << Q_FUNC_INFO << " selected item " << item << ", prev " << prev;
    if(currentGroup) emit groupUnselected(currentGroup);
    currentGroup = forumGroups.value(item);
    emit groupSelected(currentGroup);
}

void ForumListWidget::addSubscription(ForumSubscription *sub) {
    if(listWidgets.contains(sub)) return; // Already exists
    if(!sub || !sub->updateEngine()) {
        qDebug() << Q_FUNC_INFO << "Invalid subscription - ignoring.";
        return;
    }

    QListWidget *lw = new QListWidget(this);
    listWidgets.insert(sub, lw);
    addItem(lw, sub->alias());
    connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)), this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
    connect(sub, SIGNAL(unreadCountChanged()), this, SLOT(updateSubscriptionLabel()));
    connect(sub, SIGNAL(changed()), this, SLOT(updateSubscriptionLabel()));
    connect(sub, SIGNAL(groupAdded(ForumGroup*)), this, SLOT(groupFound(ForumGroup*)));
    connect(sub, SIGNAL(groupRemoved(ForumGroup*)), this, SLOT(groupDeleted(ForumGroup*)));
    connect(sub, SIGNAL(destroyed(QObject*)), this, SLOT(subscriptionDeleted(QObject*)));

    Q_ASSERT(sub->updateEngine());
    connect(sub->updateEngine(), SIGNAL(stateChanged(UpdateEngine*, UpdateEngine::UpdateEngineState,UpdateEngine::UpdateEngineState)),
            this, SLOT(updateEngineStateChanged(UpdateEngine*, UpdateEngine::UpdateEngineState)));

    setupFavicon(sub);

    for(ForumGroup *grp : sub->values())
        groupFound(grp);

    updateSubscriptionLabel(sub);
}

void ForumListWidget::subscriptionChanged(ForumSubscription *sub) {
    updateSubscriptionLabel(sub);
}

void ForumListWidget::setupFavicon(ForumSubscription *sub) {
    QUrl fiUrl = sub->faviconUrl();
    if(!fiUrl.isValid()) return; // No parser -> no url known yet
    Q_ASSERT(listWidgets.value(sub));
    Favicon *fi = new Favicon(listWidgets.value(sub), sub); // Deleted automatically when listWidget deleted
    connect(fi, SIGNAL(iconChanged(ForumSubscription*, QIcon)), this, SLOT(iconUpdated(ForumSubscription*, QIcon)));
    fi->fetchIcon(fiUrl, QPixmap(":/data/emblem-web.png"));
    forumIcons.insert(sub, fi);
}

void ForumListWidget::groupFound(ForumGroup *grp) {
    Q_ASSERT(grp);
    connect(grp, SIGNAL(changed()), this, SLOT(groupChanged()));
    connect(grp, SIGNAL(unreadCountChanged()), this, SLOT(updateGroupLabel()));
    connect(grp, SIGNAL(destroyed(QObject*)), this, SLOT(groupDestroyed(QObject*)));

    if(!grp->isSubscribed()) return;
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *lwi = new QListWidgetItem();
    lwi->setIcon(QIcon(":/data/folder.png"));
    lw->addItem(lwi);
    forumGroups.insert(lwi, grp);
    groupChanged(grp); // Set title etc.
}

void ForumListWidget::groupChanged() {
    ForumGroup *grp = static_cast<ForumGroup*> (sender());
    groupChanged(grp);
}

void ForumListWidget::groupChanged(ForumGroup *grp) {
    Q_ASSERT(grp);
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *gItem = forumGroups.key(grp);
    if(gItem && !grp->isSubscribed()) {
        // delete unsubscribed group from UI
        groupDeleted(grp);
    } else if(gItem && grp->isSubscribed()) {
        updateGroupLabel(grp);
    } else if(!gItem && grp->isSubscribed()) {
        // Add subscribed group to UI
        groupFound(grp);
    }
}

void ForumListWidget::updateSubscriptionLabel() {
    ForumSubscription *sub = static_cast<ForumSubscription*> (sender());
    updateSubscriptionLabel(sub);
}

void ForumListWidget::updateSubscriptionLabel(ForumSubscription* sub) {
    QListWidget *lw = listWidgets.value(sub);
    // Update subscription message count
    QString title = sub->alias();

    if(sub->unreadCount() > 0)
        title = QString("%1 (%2)").arg(title).arg(sub->unreadCount());
#ifdef DEBUG_INFO
        if(sub->beingSynced())
            title += "(S)";
        if(sub->scheduledForUpdate())
            title += "(u)";
        if(sub->beingUpdated())
            title += "(U)";
#endif

    setItemText(indexOf(lw), title);
}

void ForumListWidget::updateGroupLabel() {
    ForumGroup *grp = static_cast<ForumGroup*> (sender());
    updateGroupLabel(grp);
}

void ForumListWidget::updateGroupLabel(ForumGroup* grp) {
    Q_ASSERT(grp);
    if(!grp->isSubscribed()) return;
    if(!grp->subscription()) return; // May happen while quittinq
    QListWidget *lw = listWidgets.value(grp->subscription());
    Q_ASSERT(lw);
    QListWidgetItem *gItem = forumGroups.key(grp);
    if(gItem) { // may not exist in some situ
        QString title = grp->displayName();

        if (grp->unreadCount() > 0)
            title = title + " (" + QString().number(grp->unreadCount()) + ")";
#ifdef DEBUG_INFO
        if(grp->hasChanged())
            title += " (C)";
#endif
        if(!grp->hierarchy().isEmpty())
            title += " [" + grp->hierarchy() + "]";
        gItem->setText(title);
    }
}

void ForumListWidget::groupDestroyed(QObject* g) {
    ForumGroup *grp = dynamic_cast<ForumGroup*> (g);
    if(grp) groupDeleted(grp);
}

// Called also after unsubscribing - just delete the group from list if it is there
void ForumListWidget::groupDeleted(ForumGroup *grp) {
    // Change group if it's the current..
    if(currentGroup == grp) {
        currentGroup = 0;
        emit groupSelected(0);
    }
    // Do we have the item visible?

    QListWidgetItem *item = forumGroups.key(grp);
    if(item) {
        forumGroups.remove(item);
        delete item; // Should also remove it from list
    }
}

void ForumListWidget::subscriptionDeleted(QObject* qo) {
    ForumSubscription *sub = static_cast<ForumSubscription*>(qo);
    if(sub) subscriptionDeleted(sub);
}

void ForumListWidget::subscriptionDeleted(ForumSubscription* sub) {
    if(currentGroup && currentGroup->subscription()==sub) {
        currentGroup = 0;
        emit groupSelected(0);
        emit forumSelected(0);
    }
    QListWidget *lw = listWidgets.value(sub);
    if(lw) { // Sometimes may not
        listWidgets.remove(sub);
        // FavIcon deleted automatically here
        lw->deleteLater(); // Removes it from list
    }
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
        QString grpName = currentGroup->name();
        msgBox.setText("Really unsubscribe from " + MessageFormatting::stripHtml(grpName) + "?");
        msgBox.setInformativeText(currentGroup->name());
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec()==QMessageBox::Yes)
            emit unsubscribeGroup(currentGroup);
    }
}

void ForumListWidget::markAllReadClicked(bool un) {
    if(currentGroup) {
        for(ForumThread *thread : currentGroup->values()) {
            for(ForumMessage *msg : thread->values()) {
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

void ForumListWidget::updateEngineStateChanged(UpdateEngine*,UpdateEngine::UpdateEngineState state) {
    Q_UNUSED(state);
    UpdateEngine *engine = qobject_cast<UpdateEngine*>(sender());
    if(!engine->subscription()) return;
    if(!forumIcons.contains(engine->subscription()) && (engine->state() != UpdateEngine::UES_ENGINE_NOT_READY)) {
        setupFavicon(engine->subscription());
    }
}
