#include "forumlistwidget.h"
#include "favicon.h"
#include <siilihai/messageformatting.h>
#include <siilihai/forumthread.h>
#include <siilihai/forummessage.h>
#include <siilihai/parserdatabase.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/forumsubscription.h>
#include <siilihai/forumgroup.h>


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
    QListWidget *curWidget = dynamic_cast<QListWidget*> (currentWidget());
    ForumSubscription *sub = 0;
    ForumGroup *g = 0;
    if(i >= 0) {
        sub = listWidgets.key(dynamic_cast<QListWidget*> (widget(i)));

        foreach(ForumGroup *grp, sub->values()) {
            if(!g && grp->isSubscribed())
                g = grp;
        }
        dynamic_cast<QListWidget*> (widget(i))->setCurrentRow(0);
    }
    emit forumSelected(sub);
    emit groupSelected(g);
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
    // qDebug() << Q_FUNC_INFO << " selected item " << item << ", prev " << prev;
    if(currentGroup) emit groupUnselected(currentGroup);
    currentGroup = forumGroups.value(item);
    emit groupSelected(currentGroup);
}

void ForumListWidget::addSubscription(ForumSubscription *sub) {
    if(listWidgets.contains(sub)) return; // Already exists
    QListWidget *lw = new QListWidget(this);
    listWidgets.insert(sub, lw);
    addItem(lw, sub->alias());
    connect(lw, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem *)), this, SLOT(groupSelected(QListWidgetItem*,QListWidgetItem *)));
    connect(sub, SIGNAL(unreadCountChanged()), this, SLOT(updateSubscriptionLabel()));
    connect(sub, SIGNAL(groupAdded(ForumGroup*)), this, SLOT(groupFound(ForumGroup*)));
    connect(sub, SIGNAL(groupRemoved(ForumGroup*)), this, SLOT(groupDeleted(ForumGroup*)));
    connect(sub, SIGNAL(destroyed(QObject*)), this, SLOT(subscriptionDeleted(QObject*)));

    Q_ASSERT(sub->parserEngine());
    connect(sub->parserEngine(), SIGNAL(stateChanged(ParserEngine*,ParserEngine::ParserEngineState,ParserEngine::ParserEngineState)),
            this, SLOT(parserEngineStateChanged(ParserEngine*,ParserEngine::ParserEngineState)));

    if(sub->parserEngine()->parser())
        setupFavicon(sub);

    foreach(ForumGroup *grp, sub->values())
        groupFound(grp);
    updateSubscriptionLabel(sub);
}

void ForumListWidget::subscriptionChanged(ForumSubscription *sub) {
    updateSubscriptionLabel(sub);
}

void ForumListWidget::setupFavicon(ForumSubscription *sub) {
    QString fiUrl = sub->parserEngine()->parser()->forum_url;
    fiUrl = fiUrl.replace(QUrl(fiUrl).path(), "");
    fiUrl = fiUrl + "/favicon.ico";
    Q_ASSERT(listWidgets.value(sub));
    Favicon *fi = new Favicon(listWidgets.value(sub), sub); // Deleted automatically when listWidget deleted
    connect(fi, SIGNAL(iconChanged(ForumSubscription*, QIcon)), this, SLOT(iconUpdated(ForumSubscription*, QIcon)));
    fi->fetchIcon(QUrl(QUrl(fiUrl)), QPixmap(":/data/emblem-web.png"));
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
        lw->takeItem(lw->row(gItem));
        forumGroups.remove(gItem);
        delete gItem;
        if(currentGroup == grp) {
            currentGroup = 0;
            emit groupSelected(currentGroup);
        }
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
        gItem->setText(title);
    }
}

void ForumListWidget::groupDestroyed(QObject* g) {
    ForumGroup *grp = dynamic_cast<ForumGroup*> (g);
    if(grp) groupDeleted(grp);
}

void ForumListWidget::groupDeleted(ForumGroup *grp) {
    if(!grp->isSubscribed()) return;
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
        Q_ASSERT(lw);
        removeItem(indexOf(lw));
        listWidgets.remove(sub);
        // FavIcon deleted automatically here
        lw->deleteLater();
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
        foreach(ForumThread *thread, currentGroup->values()) {
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

void ForumListWidget::parserEngineStateChanged(ParserEngine* engine,ParserEngine::ParserEngineState state) {
    if(!engine->subscription()) return;
    if(!forumIcons.contains(engine->subscription()) && (engine->parser())) {
        setupFavicon(engine->subscription());
    }
}
