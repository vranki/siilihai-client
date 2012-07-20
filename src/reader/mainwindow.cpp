#include "mainwindow.h"
#include <siilihai/parserdatabase.h>
#include <siilihai/forumsubscription.h>
#include <siilihai/forumgroup.h>
#include <siilihai/forumthread.h>
#include <siilihai/forummessage.h>

#include "forumlistwidget.h"
#include "settingsdialog.h"
#include "threadlistwidget.h"
#include "messageviewwidget.h"
#include "forumproperties.h"
#include "threadproperties.h"
#include "useraccountdialog.h"

MainWindow::MainWindow(ForumDatabase &fd, QSettings *s, QWidget *parent) : QMainWindow(parent), fdb(fd), viewAsGroup(this) {
    ui.setupUi(this);
    offline = false;
    settings = s;
    viewAsGroup.addAction(ui.actionHTML);
    viewAsGroup.addAction(ui.actionText);
    viewAsGroup.addAction(ui.actionHTML_Source);

    ui.actionWeb_Page->setChecked(true);
    connect(ui.actionSubscribe_to, SIGNAL(triggered()), this, SLOT(subscribeForumSlot()));
    connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this, SLOT(groupSubscriptionsSlot()));
    connect(ui.actionUpdate_all, SIGNAL(triggered()), this, SLOT(updateClickedSlot()));
    connect(ui.actionUpdate_selected, SIGNAL(triggered()), this, SLOT(updateSelectedClickedSlot()));
    connect(ui.actionForce_update_on_selected, SIGNAL(triggered()), this, SLOT(forceUpdateSelectedClickedSlot()));
    connect(ui.actionReport_broken_or_working, SIGNAL(triggered()), this, SLOT(reportClickedSlot()));
    connect(ui.actionUnsubscribe, SIGNAL(triggered()), this, SLOT(unsubscribeForumSlot()));
    connect(ui.actionParser_Maker, SIGNAL(triggered()), this, SLOT(launchParserMakerSlot()));
    connect(ui.actionAbout_Siilihai, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(settingsDialog()));
    connect(ui.actionMark_forum_as_read, SIGNAL(triggered()), this, SLOT(markForumRead()));
    connect(ui.actionMark_forum_as_unread, SIGNAL(triggered()), this, SLOT(markForumUnread()));
    connect(ui.actionMark_group_as_Read, SIGNAL(triggered()), this, SLOT(markGroupRead()));
    connect(ui.actionMark_group_as_Unread, SIGNAL(triggered()), this, SLOT(markGroupUnread()));
    connect(ui.actionWork_offline, SIGNAL(toggled(bool)), this, SLOT(offlineClickedSlot()));
    connect(ui.actionUserAccount, SIGNAL(triggered()), this, SLOT(userAccountSettings()));
    connect(ui.actionForumProperties, SIGNAL(triggered()), this, SLOT(forumPropertiesSlot()));
    connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
    connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
    connect(ui.hideButton, SIGNAL(clicked()), this, SLOT(hideClickedSlot()));
    connect(ui.viewInBrowser, SIGNAL(clicked()), this, SLOT(viewInBrowserClickedSlot()));
    flw = new ForumListWidget(this);
    connect(flw, SIGNAL(groupSelected(ForumGroup*)), this, SLOT(groupSelected(ForumGroup*)));
    connect(flw, SIGNAL(forumSelected(ForumSubscription*)), this, SLOT(forumSelected(ForumSubscription*)));
    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), flw, SLOT(addSubscription(ForumSubscription*)));
    connect(flw, SIGNAL(groupUnselected(ForumGroup*)), this, SIGNAL(groupUnselected(ForumGroup*)));

    ui.forumsSplitter->insertWidget(0, flw);
    tlw = new ThreadListWidget(this);
    connect(flw, SIGNAL(groupSelected(ForumGroup*)), tlw, SLOT(groupSelected(ForumGroup*)));
    connect(flw, SIGNAL(unsubscribeGroup(ForumGroup*)), this, SIGNAL(unsubscribeGroup(ForumGroup*)));
    connect(flw, SIGNAL(groupSubscriptions(ForumSubscription*)), this, SIGNAL(groupSubscriptions(ForumSubscription*)));
    connect(flw, SIGNAL(unsubscribeForum()), this, SLOT(unsubscribeForumSlot()));
    connect(flw, SIGNAL(forumProperties()), this, SLOT(forumPropertiesSlot()));
    connect(tlw, SIGNAL(messageSelected(ForumMessage*)), this, SLOT(messageSelected(ForumMessage*)));
    connect(tlw, SIGNAL(moreMessagesRequested(ForumThread*)), this, SIGNAL(moreMessagesRequested(ForumThread*)));
    connect(tlw, SIGNAL(viewInBrowser()), this, SLOT(viewInBrowserClickedSlot()));
    connect(tlw, SIGNAL(threadProperties(ForumThread *)), this, SLOT(threadPropertiesSlot(ForumThread *)));
    connect(tlw, SIGNAL(updateThread(ForumThread*,bool)), this, SIGNAL(updateThread(ForumThread*,bool)));
    connect(ui.nextUnreadButton, SIGNAL(clicked()), tlw, SLOT(selectNextUnread()));
    ui.topFrame->layout()->addWidget(tlw);

    mvw = new MessageViewWidget(this);
    ui.horizontalSplitter->addWidget(mvw);
    connect(tlw, SIGNAL(messageSelected(ForumMessage*)), mvw, SLOT(messageSelected(ForumMessage*)));
    connect(ui.actionHTML, SIGNAL(triggered()), mvw, SLOT(viewAsHTML()));
    connect(ui.actionText, SIGNAL(triggered()), mvw, SLOT(viewAsText()));
    connect(ui.actionHTML_Source, SIGNAL(triggered()), mvw, SLOT(viewAsSource()));
    ui.actionHTML->setChecked(true);

    connect(&fdb, SIGNAL(subscriptionFound(ForumSubscription*)), this, SLOT(subscriptionFound(ForumSubscription*)));

    flw->installEventFilter(this);
    tlw->installEventFilter(this);
    mvw->installEventFilter(this);

    if (!restoreGeometry(settings->value("reader_geometry").toByteArray()))
        showMaximized();
    ui.forumsSplitter->restoreState(
            settings->value("reader_splitter_size").toByteArray());
    ui.horizontalSplitter->restoreState(settings->value(
            "reader_horizontal_splitter_size").toByteArray());
#ifdef Q_WS_HILDON
    ui.updateButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui.stopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui.hideButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui.viewInBrowser->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tlw->setHeaderHidden(true);
    hideClickedSlot();
    hideClickedSlot();
#else
    ui.hideButton->hide();
#endif
#ifdef DEBUG_INFO
    setWindowTitle(windowTitle() + " (Debug Build)");
#endif
    foreach(ForumSubscription *sub, fdb.values())
        flw->addSubscription(sub);
}

MainWindow::~MainWindow() {
}

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

void MainWindow::subscribeForumSlot() {
    emit subscribeForum();
}

void MainWindow::launchParserMakerSlot() {
    emit launchParserMaker();
}

void MainWindow::updateClickedSlot() {
    emit updateClicked();
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

void MainWindow::cancelClickedSlot() {
    emit cancelClicked();
}

void MainWindow::groupSubscriptionsSlot() {
    emit groupSubscriptions(flw->getSelectedForum());
}

void MainWindow::viewInBrowserClickedSlot() {
    if (mvw->currentMessage())
        QDesktopServices::openUrl(mvw->currentMessage()->url());
}

// Caution - engine->subscription() may be null (when deleted)!
void MainWindow::parserEngineStateChanged(ParserEngine *engine, ParserEngine::ParserEngineState newState, ParserEngine::ParserEngineState oldState) {
    if (newState==ParserEngine::PES_UPDATING) {
        busyParserEngines.insert(engine);
    } else {
        busyParserEngines.remove(engine);
    }
    updateEnabledButtons();
}

void MainWindow::setOffline(bool readerOffline) {
    offline = readerOffline;
    ui.actionWork_offline->setChecked(offline);
    updateEnabledButtons();
}

void MainWindow::about() {
    QString aboutText = "<h1>Siilihai</h1><p> by Ville Ranki &lt;ville.ranki@iki.fi&gt;</p> "
                        "<p>Artwork by Gnome project and SJ</p><p>Released under GNU GPLv3</p>"
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

void MainWindow::messageSelected(ForumMessage *msg) {
    ui.viewInBrowser->setEnabled(msg != 0);
}


void MainWindow::updateEnabledButtons() {
    ui.updateButton->setEnabled(!offline);
    ui.actionUpdate_all->setEnabled(!offline);
    ui.actionSubscribe_to->setEnabled(!offline);

    ui.stopButton->setEnabled(!busyParserEngines.isEmpty());
    ui.updateButton->setEnabled(busyParserEngines.isEmpty() && !offline);

    bool sane = (flw->getSelectedForum() != 0);
    ui.actionUpdate_selected->setEnabled( sane && !offline);
    ui.actionForce_update_on_selected->setEnabled( sane && !offline);
    ui.actionReport_broken_or_working->setEnabled(sane && !offline);
    ui.actionUnsubscribe->setEnabled( sane && !offline);
    ui.actionGroup_Subscriptions->setEnabled(sane && !offline);
    ui.actionUpdate_selected->setEnabled( sane && !offline);
    ui.actionMark_forum_as_read->setEnabled(sane);
    ui.actionMark_forum_as_unread->setEnabled(sane);

    sane = (flw->getSelectedGroup() != 0);
    ui.actionMark_group_as_Read->setEnabled(sane);
    ui.actionMark_group_as_Unread->setEnabled(sane);
}

void MainWindow::forumSelected(ForumSubscription *sub) {
    updateEnabledButtons();
}

void MainWindow::groupSelected(ForumGroup *fg) {
    if(!fg) return;
#ifdef Q_WS_HILDON
    hideClickedSlot();
#endif
    updateEnabledButtons();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
 {
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
    connect(sub->parserEngine(), SIGNAL(stateChanged(ParserEngine*,ParserEngine::ParserEngineState,ParserEngine::ParserEngineState)),
            this, SLOT(parserEngineStateChanged(ParserEngine*,ParserEngine::ParserEngineState,ParserEngine::ParserEngineState)));
}

void MainWindow::showMessage(QString msg, int time) {
    ui.statusbar->showMessage(msg, time);
}
