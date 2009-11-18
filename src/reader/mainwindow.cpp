#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QSettings *s,
		QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	readerReady = false;
	settings = s;
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
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(cancelClickedSlot()));
	connect(ui.hideButton, SIGNAL(clicked()), this, SLOT(hideClickedSlot()));
	connect(ui.viewInBrowser, SIGNAL(clicked()), this,
			SLOT(viewInBrowserClickedSlot()));
	flw = new ForumListWidget(this, fdb, pdb);
	ui.forumsSplitter->insertWidget(0, flw);
	flw->setEnabled(false);
	tlw = new ThreadListWidget(this, fdb);
	ui.topFrame->layout()->addWidget(tlw);

	// ui.horizontalSplitter->insertWidget(1,tlw);
	connect(flw, SIGNAL(groupSelected(ForumGroup)), tlw,
			SLOT(groupSelected(ForumGroup)));
	connect(flw, SIGNAL(groupSelected(ForumGroup)), this,
			SLOT(groupSelected(ForumGroup)));
	connect(tlw, SIGNAL(messageSelected(const ForumMessage&)), this,
			SLOT(messageSelected(const ForumMessage&)));

	mvw = new MessageViewWidget(this);
	ui.horizontalSplitter->addWidget(mvw);
	connect(tlw, SIGNAL(messageSelected(const ForumMessage&)), mvw,
			SLOT(messageSelected(const ForumMessage&)));

	if (!restoreGeometry(settings->value("reader_geometry").toByteArray()))
		showMaximized();
	ui.forumsSplitter->restoreState(settings->value("reader_splitter_size").toByteArray());
	ui.horizontalSplitter->restoreState(settings->value("reader_horizontal_splitter_size").toByteArray());
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
	settings->setValue("reader_horizontal_splitter_size", ui.horizontalSplitter->saveState());
	event->accept();
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
	int selectedForum = flw->getSelectedForum().parser;
	emit reportClicked(selectedForum);
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
	int selectedForum = flw->getSelectedForum().parser;
	if (selectedForum > 0)
		emit updateClicked(selectedForum, false);
}

void MainWindow::forceUpdateSelectedClickedSlot() {
	int selectedForum = flw->getSelectedForum().parser;
	if (selectedForum > 0)
		emit updateClicked(selectedForum, true);
}

void MainWindow::unsubscribeForumSlot() {
	int selectedForum = flw->getSelectedForum().parser;
	if (selectedForum > 0)
		emit unsubscribeForum(selectedForum);
}

void MainWindow::cancelClickedSlot() {
	emit cancelClicked();
}

void MainWindow::groupSubscriptionsSlot() {
	int selectedForum = flw->getSelectedForum().parser;
	if (selectedForum > 0)
		emit groupSubscriptions(selectedForum);
}

void MainWindow::viewInBrowserClickedSlot() {
	if (mvw->currentMessage().isSane())
		QDesktopServices::openUrl(mvw->currentMessage().url);
}

void MainWindow::setForumStatus(int forum, bool reloading, float progress) {
	if (reloading) {
		busyForums.insert(forum);
	} else {
		busyForums.remove(forum);
	}
	ui.stopButton->setEnabled(!busyForums.isEmpty() && readerReady);
	ui.updateButton->setEnabled(busyForums.isEmpty() && readerReady);
	if (!busyForums.isEmpty()) {
		ui.statusbar->showMessage("Updating Forums, please stand by.. ", 5000);
	} else {
		ui.statusbar->showMessage("Forums updated", 5000);
	}

	flw->setForumStatus(forum, reloading, progress);
}

ForumListWidget* MainWindow::forumList() {
	return flw;
}

ThreadListWidget* MainWindow::threadList() {
	return tlw;
}

void MainWindow::setReaderReady(bool ready, bool offline) {
	readerReady = ready;
	ui.updateButton->setEnabled(readerReady && !offline);
	flw->setEnabled(readerReady || offline);
	if (!ready) {
		ui.statusbar->showMessage("Starting up, please wait..", 2000);
	} else {
		if(!offline) {
			ui.statusbar->showMessage("Siilihai is ready", 2000);
		} else {
			ui.statusbar->showMessage("Siilihai is ready, but in offline mode", 2000);
		}
	}
	if(ready && !offline) {
		ui.updateButton->setEnabled(true);
		ui.actionUnsubscribe->setEnabled(true);
		ui.actionUpdate_all->setEnabled(true);
		ui.actionUpdate_selected->setEnabled(true);
		ui.actionGroup_Subscriptions->setEnabled(true);
		ui.actionSubscribe_to->setEnabled(true);
		ui.actionReport_broken_or_working->setEnabled(true);
	}
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
	ForumSubscription selectedForum = flw->getSelectedForum();

	if (selectedForum.isSane()) {
		fdb.markForumRead(selectedForum.parser, read);
		tlw->groupSelected(ForumGroup());
		flw->updateForumList();
	}
}

void MainWindow::markForumUnread() {
	markForumRead(false);
}

void MainWindow::markGroupRead(bool read) {
	qDebug() << Q_FUNC_INFO;
	ForumGroup selectedGroup = flw->getSelectedGroup();

	if (selectedGroup.isSane()) {
		fdb.markGroupRead(selectedGroup, read);
		tlw->groupSelected(selectedGroup);
		flw->updateForumList();
	}
}

void MainWindow::markGroupUnread() {
	markGroupRead(false);
}

void MainWindow::messageSelected(const ForumMessage &msg) {
	if (msg.isSane()) {
		ui.viewInBrowser->setEnabled(true);
		flw->updateReadCounts();
	}
}

void MainWindow::groupSelected(ForumGroup fg) {
#ifdef Q_WS_HILDON
	hideClickedSlot();
#endif
	bool saneGroup = fg.isSane();
	ui.actionMark_group_as_Read->setEnabled(saneGroup);
	ui.actionMark_group_as_Unread->setEnabled(saneGroup);
}
