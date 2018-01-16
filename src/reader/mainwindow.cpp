#include "mainwindow.h"
#include <siilihai/parser/parserdatabase.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forummessage.h>

#include "forumlistwidget.h"
#include "settingsdialog.h"
#include "threadlistwidget.h"
#include "messageviewwidget.h"
#include "forumproperties.h"
#include "threadproperties.h"
#include "useraccountdialog.h"
#include "composemessage.h"

MainWindow::MainWindow(ForumDatabase &fd, SiilihaiSettings *s, QWidget *parent)
    : QMainWindow(parent)
    , fdb(fd) {
    ui.setupUi(this);
    offline = false;
    settings = s;

    connect(ui.actionSubscribe_to, SIGNAL(triggered()), this, SIGNAL(subscribeForum()));
    connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this, SLOT(groupSubscriptionsSlot()));
    connect(ui.actionUpdate_all, SIGNAL(triggered()), this, SIGNAL(updateClicked()));
    connect(ui.actionUpdate_selected, SIGNAL(triggered()), this, SLOT(updateSelectedClickedSlot()));
    connect(ui.actionForce_update_on_selected, SIGNAL(triggered()), this, SLOT(forceUpdateSelectedClickedSlot()));
    connect(ui.actionReport_broken_or_working, SIGNAL(triggered()), this, SLOT(reportClickedSlot()));
    connect(ui.actionUnsubscribe, SIGNAL(triggered()), this, SLOT(unsubscribeForumSlot()));
    connect(ui.actionParser_Maker, SIGNAL(triggered()), this, SIGNAL(launchParserMaker()));
    connect(ui.actionAbout_Siilihai, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(settingsDialog()));
    connect(ui.actionMark_forum_as_read, SIGNAL(triggered()), this, SLOT(markForumRead()));
    connect(ui.actionMark_forum_as_unread, SIGNAL(triggered()), this, SLOT(markForumUnread()));
    connect(ui.actionMark_group_as_Read, SIGNAL(triggered()), this, SLOT(markGroupRead()));
    connect(ui.actionMark_group_as_Unread, SIGNAL(triggered()), this, SLOT(markGroupUnread()));
    connect(ui.actionWork_offline, SIGNAL(toggled(bool)), this, SLOT(offlineClickedSlot()));
    connect(ui.actionUserAccount, SIGNAL(triggered()), this, SLOT(userAccountSettings()));
    connect(ui.actionForumProperties, SIGNAL(triggered()), this, SLOT(forumPropertiesSlot()));
    connect(ui.updateButton, SIGNAL(clicked()), this, SIGNAL(updateClicked()));
    connect(ui.stopButton, SIGNAL(clicked()), this, SIGNAL(cancelClicked()));
    connect(ui.hideButton, SIGNAL(clicked()), this, SLOT(hideClickedSlot()));
    connect(ui.viewInBrowser, SIGNAL(clicked()), this, SLOT(viewInBrowserClickedSlot()));
    connect(ui.actionUpdate_all_parsers, SIGNAL(triggered()), this, SIGNAL(updateAllParsers()));
    connect(ui.newThreadButton, SIGNAL(clicked()), this, SLOT(newThreadClickedSlot()));
    connect(ui.replyButton, SIGNAL(clicked()), this, SLOT(replyClickedSlot()));
    connect(ui.replyQuoted, SIGNAL(clicked()), this, SLOT(replyQuotedClickedSlot()));
    mvw = new MessageViewWidget(this);

    flw = new ForumListWidget(this);
    connect(flw, SIGNAL(groupSelected(ForumGroup*)), this, SLOT(updateEnabledButtons()));
    connect(flw, SIGNAL(forumSelected(ForumSubscription*)), this, SLOT(updateEnabledButtons()));
    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), flw, SLOT(addSubscription(ForumSubscription*)));
    connect(flw, SIGNAL(groupUnselected(ForumGroup*)), this, SIGNAL(groupUnselected(ForumGroup*)));

    ui.forumsSplitter->insertWidget(0, flw);
    tlw = new ThreadListWidget(this);
    connect(flw, SIGNAL(groupSelected(ForumGroup*)), tlw, SLOT(groupSelected(ForumGroup*)));
    connect(flw, SIGNAL(unsubscribeGroup(ForumGroup*)), this, SIGNAL(unsubscribeGroup(ForumGroup*)));
    connect(flw, SIGNAL(groupSubscriptions(ForumSubscription*)), this, SIGNAL(groupSubscriptions(ForumSubscription*)));
    connect(flw, SIGNAL(unsubscribeForum()), this, SLOT(unsubscribeForumSlot()));
    connect(flw, SIGNAL(forumProperties()), this, SLOT(forumPropertiesSlot()));
    connect(flw, SIGNAL(displaySubscriptionErrors(ForumSubscription*)), mvw, SLOT(displaySubscriptionErrors(ForumSubscription*)));
    connect(mvw, SIGNAL(currentMessageChanged(ForumMessage*)), this, SLOT(updateEnabledButtons()));

    connect(tlw, SIGNAL(moreMessagesRequested(ForumThread*)), this, SIGNAL(moreMessagesRequested(ForumThread*)));
    connect(tlw, SIGNAL(viewInBrowser()), this, SLOT(viewInBrowserClickedSlot()));
    connect(tlw, SIGNAL(threadProperties(ForumThread *)), this, SLOT(threadPropertiesSlot(ForumThread *)));
    connect(tlw, SIGNAL(updateThread(ForumThread*,bool)), this, SIGNAL(updateThread(ForumThread*,bool)));
    connect(ui.nextUnreadButton, SIGNAL(clicked()), tlw, SLOT(selectNextUnread()));
    ui.topFrame->layout()->addWidget(tlw);

    ui.horizontalSplitter->addWidget(mvw);
    connect(tlw, SIGNAL(messageSelected(ForumMessage*)), mvw, SLOT(messageSelected(ForumMessage*)));
    connect(ui.actionHTML_Source, SIGNAL(toggled(bool)), mvw, SLOT(viewAsSource(bool)));

    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), this, SLOT(subscriptionFound(ForumSubscription*)));

    flw->installEventFilter(this);
    tlw->installEventFilter(this);
    mvw->installEventFilter(this);

    if (!restoreGeometry(settings->value("reader_geometry").toByteArray()))
        showMaximized();
    if(settings->contains("reader_splitter_size")) {
        ui.forumsSplitter->restoreState(settings->value("reader_splitter_size").toByteArray());
    } else {
        QList<int> splitterSizes;
        splitterSizes << width() / 4 << width()*3 / 4;
        ui.forumsSplitter->setSizes(splitterSizes);
    }
    ui.horizontalSplitter->restoreState(settings->value("reader_horizontal_splitter_size").toByteArray());
    ui.hideButton->hide();
#ifdef DEBUG_INFO
    setWindowTitle(windowTitle() + " (Debug Build)");
    connect(ui.actionStart_sync, SIGNAL(triggered()), this, SIGNAL(startSyncClicked()));
    connect(ui.actionEnd_sync, SIGNAL(triggered()), this, SIGNAL(endSyncClicked()));
#else
    menuBar()->removeAction(ui.menuDebug->menuAction());
#endif
    if(settings->noAccount()) {
        ui.actionReport_broken_or_working->setEnabled(false);
    }

    for(ForumSubscription *sub : fdb)
        flw->addSubscription(sub);
}

MainWindow::~MainWindow() { }

void MainWindow::closeEvent(QCloseEvent *event) {
    settings->setValue("reader_geometry", saveGeometry());
    settings->setValue("reader_splitter_size", ui.forumsSplitter->saveState());
    settings->setValue("reader_horizontal_splitter_size", ui.horizontalSplitter->saveState());
    event->ignore();
    emit haltRequest();
}

void MainWindow::offlineClickedSlot() {
    emit offlineModeSet(ui.actionWork_offline->isChecked());
}

void MainWindow::reportClickedSlot() {
    emit reportClicked(flw->getSelectedForum());
}

void MainWindow::hideClickedSlot() {
    if (flw->isHidden()) {
        flw->show();
        ui.hideButton->setText("Hide");
        ui.hideButton->setIcon(QIcon(":/data/go-first.png"));
    } else {
        flw->hide();
        ui.hideButton->setIcon(QIcon(":/data/go-last.png"));
        ui.hideButton->setText("Show");
    }
}

void MainWindow::updateSelectedClickedSlot() {
    emit updateClicked(flw->getSelectedForum(), false);
}

void MainWindow::forceUpdateSelectedClickedSlot() {
    emit updateClicked(flw->getSelectedForum(), true);
}

void MainWindow::unsubscribeForumSlot() {
    ForumSubscription *sub = flw->getSelectedForum();
    if(sub)
        emit unsubscribeForum(sub);
}

void MainWindow::groupSubscriptionsSlot() {
    emit groupSubscriptions(flw->getSelectedForum());
}

void MainWindow::viewInBrowserClickedSlot() {
    if (mvw->currentMessage())
        QDesktopServices::openUrl(mvw->currentMessage()->url());
}

// Caution - engine->subscription() may be null (when deleted)!
void MainWindow::updateEngineStateChanged(UpdateEngine *engine, UpdateEngine::UpdateEngineState newState,
                                          UpdateEngine::UpdateEngineState oldState) {
    Q_UNUSED(oldState);
    Q_UNUSED(newState);
    Q_UNUSED(engine);
    updateEnabledButtons();
}

void MainWindow::setOffline(bool readerOffline) {
    offline = readerOffline;
    ui.actionWork_offline->setChecked(offline);
    updateEnabledButtons();
}

void MainWindow::about() {
    QString aboutText = "<h1>Siilihai</h1><p> by Ville Ranki &lt;ville.ranki@iki.fi&gt;</p> "
                        "<p>Artwork by Gnome project and SJ</p><p>Powered by TapaTalk</p><p>Released under GNU GPLv3</p>"
                        "<p>Version: <b>Development</b> (Date)</p>";
    aboutText += "<p>Settings stored to " + settings->fileName() + "</p>";
#ifdef SIILIHAI_CLIENT_VERSION
    aboutText.replace("Development", SIILIHAI_CLIENT_VERSION);
    aboutText.replace("Date", __DATE__);
#endif
    QMessageBox::about(this, "About Siilihai", aboutText);
}

void MainWindow::settingsDialog() {
    SettingsDialog *sd = new SettingsDialog(this, settings);
    connect(sd, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    sd->setModal(true);
    sd->exec();
}

void MainWindow::settingsDialogAccepted() {
    qDebug() << Q_FUNC_INFO;
    emit settingsChanged(true);
}

void MainWindow::markForumRead(bool read) {
    qDebug() << Q_FUNC_INFO;
    if (flw->getSelectedForum()) {
        flw->getSelectedForum()->markRead(read);
    }
}

void MainWindow::markForumUnread() {
    markForumRead(false);
}

void MainWindow::forumPropertiesSlot( ) {
    if (flw->getSelectedForum()) {
        ForumProperties *fp = new ForumProperties(this, flw->getSelectedForum(), fdb);
        fp->setModal(false);
        connect(fp, SIGNAL(forumUpdateNeeded(ForumSubscription*)), this, SIGNAL(forumUpdateNeeded(ForumSubscription*)));
        fp->show();
    }
}

void MainWindow::threadPropertiesSlot(ForumThread *thread ) {
    Q_ASSERT(thread);
    ThreadProperties *tp = new ThreadProperties(this, thread, fdb);
    tp->setModal(false);
    connect(tp, SIGNAL(updateNeeded(ForumSubscription*)), this, SLOT(updateSelectedClickedSlot()));
    tp->show();
}


void MainWindow::markGroupRead(bool read) {
    ForumGroup *selectedGroup = flw->getSelectedGroup();

    if (selectedGroup) {
        selectedGroup->markRead(read);
        tlw->groupSelected(selectedGroup);
    }
}

void MainWindow::markGroupUnread() {
    markGroupRead(false);
}

void MainWindow::updateEnabledButtons() {
    ui.updateButton->setEnabled(!offline);
    ui.actionUpdate_all->setEnabled(!offline);
    ui.actionSubscribe_to->setEnabled(!offline);

    bool forumsUpdating = false;
    for(ForumSubscription *sub : fdb) {
        if(sub->beingUpdated() || sub->scheduledForUpdate())
            forumsUpdating = true;
    }

    ui.stopButton->setEnabled(forumsUpdating);
    ui.updateButton->setEnabled(!offline);

    bool sane = (flw->getSelectedForum() != 0);
    ui.actionUpdate_selected->setEnabled( sane && !offline);
    ui.actionForce_update_on_selected->setEnabled( sane && !offline);
    ui.actionReport_broken_or_working->setEnabled(sane && !offline && !settings->noAccount());
    ui.actionUnsubscribe->setEnabled( sane && !offline);
    ui.actionGroup_Subscriptions->setEnabled(sane && !offline);
    ui.actionUpdate_selected->setEnabled( sane && !offline);
    ui.actionMark_forum_as_read->setEnabled(sane);
    ui.actionMark_forum_as_unread->setEnabled(sane);

    sane = (flw->getSelectedGroup() != 0);
    ui.actionMark_group_as_Read->setEnabled(sane);
    ui.actionMark_group_as_Unread->setEnabled(sane);

    if(sane && flw->getSelectedForum() && flw->getSelectedForum()->updateEngine()) {
        bool posting = flw->getSelectedForum()->updateEngine()->supportsPosting();
        ui.newThreadButton->setEnabled(posting && sane);
        ui.replyButton->setEnabled(posting && mvw->currentMessage());
        ui.replyQuoted->setEnabled(posting && mvw->currentMessage());
    }
    ui.viewInBrowser->setEnabled(mvw->currentMessage() && !mvw->currentMessage()->url().isNull());
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        if (object == tlw || object == flw || object == mvw ) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Space) {
                tlw->selectNextUnread();
                return true;
            } else
                return false;
        }
    }
    return false;
}

void MainWindow::userAccountSettings() {
    UserAccountDialog accountDialog(this, settings);
    connect(&accountDialog, SIGNAL(unregisterSiilihai()), this, SIGNAL(unregisterSiilihai()));
    accountDialog.exec();
}

void MainWindow::subscriptionFound(ForumSubscription *sub) {
    connect(sub->updateEngine(),
            &UpdateEngine::stateChanged,
            this,
            &MainWindow::updateEngineStateChanged);
}

void MainWindow::newThreadClickedSlot() {
    if(!flw->getSelectedGroup()) return;
    ComposeMessage *composition = new ComposeMessage(this, *settings);
    composition->newThreadIn(flw->getSelectedGroup());
    composition->setModal(false);
    composition->show();
}

void MainWindow::replyClickedSlot() {
    if(!mvw->currentMessage()) return;
    ComposeMessage *composition = new ComposeMessage(this, *settings);
    composition->newReplyTo(mvw->currentMessage()->thread());
    composition->setModal(false);
    composition->show();
}

void MainWindow::replyQuotedClickedSlot(){
    if(!mvw->currentMessage()) return;
    ComposeMessage *composition = new ComposeMessage(this, *settings);
    composition->newReplyTo(mvw->currentMessage());
    composition->setModal(false);
    composition->show();
}

void MainWindow::showMessage(QString msg, int time) {
    ui.statusbar->showMessage(msg, time);
}
