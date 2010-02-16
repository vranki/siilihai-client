#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QSettings *s,
		QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd), viewAsGroup(this) {
	ui.setupUi(this);
	readerReady = false;
	settings = s;
	viewAsGroup.addAction(ui.actionHTML);
	viewAsGroup.addAction(ui.actionText);
	viewAsGroup.addAction(ui.actionWeb_Page);
	ui.actionWeb_Page->setChecked(true);

	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this,
			SLOT(subscribeForumSlot()));
	connect(ui.actionGroup_Subscriptions, SIGNAL(triggered()), this,
			SLOT(groupSubscriptionsSlot()));
	connect(ui.actionUpdate_all, SIGNAL(triggered()), this,
			SLOT(updateClickedSlot()));
	connect(ui.actionUpdate_selected, SIGNAL(triggered()), this,
			SLOT(updateSelectedClickedSlot()));
	connect(ui.actionForce_update_on_selected, SIGNAL(triggered()), this,
			SLOT(forceUpdateSelectedClickedSlot()));
	connect(ui.actionReport_broken_or_working, SIGNAL(triggered()), this,
			SLOT(reportClickedSlot()));
	connect(ui.actionUnsubscribe, SIGNAL(triggered()), this,
			SLOT(unsubscribeForumSlot()));
	connect(ui.actionParser_Maker, SIGNAL(triggered()), this,
			SLOT(launchParserMakerSlot()));
	connect(ui.actionAbout_Siilihai, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionPreferences, SIGNAL(triggered()), this,
			SLOT(settingsDialog()));
	connect(ui.actionMark_forum_as_read, SIGNAL(triggered()), this,
			SLOT(markForumRead()));
	connect(ui.actionMark_forum_as_unread, SIGNAL(triggered()), this,
			SLOT(markForumUnread()));
	connect(ui.actionMark_group_as_Read, SIGNAL(triggered()), this,
			SLOT(markGroupRead()));
	connect(ui.actionMark_group_as_Unread, SIGNAL(triggered()), this,
			SLOT(markGroupUnread()));
	connect(ui.actionWork_offline, SIGNAL(toggled(bool)), this,
			SLOT(offlineClickedSlot()));
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
	connect(ui.hideButton, SIGNAL(clicked()), this, SLOT(hideClickedSlot()));
	connect(ui.viewInBrowser, SIGNAL(clicked()), this,
			SLOT(viewInBrowserClickedSlot()));
	flw = new ForumListWidget(this, fdb, pdb);
        connect(flw, SIGNAL(groupSelected(ForumGroup*)), this,
                        SLOT(groupSelected(ForumGroup*)));
        connect(flw, SIGNAL(forumSelected(ForumSubscription*)), this,
                        SLOT(forumSelected(ForumSubscription*)));
        ui.forumsSplitter->insertWidget(0, flw);
	flw->setEnabled(false);
	tlw = new ThreadListWidget(this, fdb);
        connect(flw, SIGNAL(groupSelected(ForumGroup*)), tlw,
                        SLOT(groupSelected(ForumGroup*)));
        connect(tlw, SIGNAL(messageSelected(ForumMessage*)), this,
                        SLOT(messageSelected(ForumMessage*)));
        ui.topFrame->layout()->addWidget(tlw);


	mvw = new MessageViewWidget(this);
	ui.horizontalSplitter->addWidget(mvw);
	connect(tlw, SIGNAL(messageSelected(ForumMessage*)), mvw,
			SLOT(messageSelected(ForumMessage*)));

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
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent *event) {
	settings->setValue("reader_geometry", saveGeometry());
	settings->setValue("reader_splitter_size", ui.forumsSplitter->saveState());
	settings->setValue("reader_horizontal_splitter_size",
			ui.horizontalSplitter->saveState());
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
	emit unsubscribeForum(flw->getSelectedForum());
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

void MainWindow::setForumStatus(ForumSubscription *forum, bool reloading, float progress) {
	if (reloading) {
		busyForums.insert(forum);
	} else {
		busyForums.remove(forum);
	}
	if (!busyForums.isEmpty()) {
		ui.statusbar->showMessage("Updating Forums, please stand by.. ", 5000);
	} else {
		ui.statusbar->showMessage("Forums updated", 5000);
	}

	flw->setForumStatus(forum, reloading, progress);
        updateEnabledButtons();
}

ForumListWidget* MainWindow::forumList() {
	return flw;
}

ThreadListWidget* MainWindow::threadList() {
	return tlw;
}

void MainWindow::setReaderReady(bool ready, bool readerOffline) {
	qDebug() << Q_FUNC_INFO << ready << readerOffline;
	readerReady = ready;
	offline = readerOffline;
	ui.actionWork_offline->setChecked(offline);
	flw->setEnabled(readerReady || offline);
	if (!ready) {
		ui.statusbar->showMessage("Starting up, please wait..", 2000);
	} else {
		if (!offline) {
			ui.statusbar->showMessage("Siilihai is ready", 2000);
		} else {
			ui.statusbar->showMessage("Siilihai is ready, but in offline mode",
					2000);
		}
	}
        updateEnabledButtons();
}

void MainWindow::about() {
	QMessageBox::about(this, "About Siilihai",
			"<h1>Siilihai</h1><p> by Ville Ranki &lt;ville.ranki@iki.fi&gt;</p> "
				"<p>Artwork by Gnome project and SJ</p><p>Released under GNU GPLv3</p>");
}

void MainWindow::settingsDialog() {
	SettingsDialog *sd = new SettingsDialog(this, settings);
	sd->setModal(true);
	sd->exec();
}

void MainWindow::markForumRead(bool read) {
	qDebug() << Q_FUNC_INFO;
	if (flw->getSelectedForum()) {
		fdb.markForumRead(flw->getSelectedForum(), read);
	}
}

void MainWindow::markForumUnread() {
	markForumRead(false);
}

void MainWindow::markGroupRead(bool read) {
	qDebug() << Q_FUNC_INFO;
	ForumGroup *selectedGroup = flw->getSelectedGroup();

	if (selectedGroup) {
		fdb.markGroupRead(selectedGroup, read);
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
	ui.updateButton->setEnabled(readerReady && !offline);
	ui.actionUpdate_all->setEnabled(readerReady && !offline);
	ui.actionSubscribe_to->setEnabled(readerReady && !offline);

	ui.stopButton->setEnabled(!busyForums.isEmpty() && readerReady);
	ui.updateButton->setEnabled(busyForums.isEmpty() && readerReady && !offline);

        bool sane = (flw->getSelectedForum() != 0);
        ui.actionUpdate_selected->setEnabled(readerReady && sane && !offline);
        ui.actionForce_update_on_selected->setEnabled(readerReady && sane && !offline);
        ui.actionReport_broken_or_working->setEnabled(readerReady && sane && !offline);
        ui.actionUnsubscribe->setEnabled(readerReady && sane && !offline);
        ui.actionGroup_Subscriptions->setEnabled(readerReady && sane && !offline);
        ui.actionUpdate_selected->setEnabled(readerReady && sane && !offline);
        ui.actionMark_forum_as_read->setEnabled(readerReady && sane);
        ui.actionMark_forum_as_unread->setEnabled(readerReady && sane);

        sane = (flw->getSelectedGroup() != 0);
        ui.actionMark_group_as_Read->setEnabled(readerReady && sane);
        ui.actionMark_group_as_Unread->setEnabled(readerReady && sane);
    }

void MainWindow::forumSelected(ForumSubscription *sub) {
    qDebug() << Q_FUNC_INFO << sub << readerReady << offline;
    updateEnabledButtons();
}

void MainWindow::groupSelected(ForumGroup *fg) {
    qDebug() << Q_FUNC_INFO << fg;
#ifdef Q_WS_HILDON
    hideClickedSlot();
#endif
    updateEnabledButtons();
}
