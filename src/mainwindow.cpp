#include "mainwindow.h"

MainWindow::MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent) :
	QMainWindow(parent), fdb(fd), pdb(pd) {
	ui.setupUi(this);
	connect(ui.actionSubscribe_to, SIGNAL(triggered()), this, SLOT(subscribeForumSlot()));
}

void MainWindow::updateForumList() {
	while (ui.forumToolBox->count() > 0) {
		ui.forumToolBox->removeItem(0);
	}
	QList<ForumSubscription> forums = fdb.listSubscriptions();
	for (int i = 0; i < forums.size(); i++) {
		ui.forumToolBox->addItem(new QLabel("Groups goes here"), forums[i].name);
	}
}

MainWindow::~MainWindow() {

}
void MainWindow::subscribeForumSlot() {
	emit subscribeForum();
}
