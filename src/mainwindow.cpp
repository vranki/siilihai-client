#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this, SLOT(subscribeForumSlot()));
	connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(updateClickedSlot()));

}

void MainWindow::updateForumList() {
	while (ui.forumToolBox->count() > 0) {
		ui.forumToolBox->removeItem(0);
	}
	forumItems.clear();
	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		ui.forumToolBox->addItem(new QLabel("Groups goes here"), forums[i].name);
		ui.forumToolBox->setItemIcon(i, QIcon("data/emblem-web.svg"));
		forumItems[forums[i].parser] = i;
	}
}

MainWindow::~MainWindow() {

}

void MainWindow::subscribeForumSlot() {
	emit subscribeForum();
}

void MainWindow::updateClickedSlot() {
	emit updateClicked();
}

void MainWindow::setForumStatus(int forum, bool reloading) {
	if(reloading) {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon("data/stock_refresh.svg"));
	} else {
		ui.forumToolBox->setItemIcon(forumItems[forum], QIcon("data/emblem-web.svg"));
	}
}
