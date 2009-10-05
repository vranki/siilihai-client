#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QList>

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>
#include <forumgroup.h>
#include <forumthread.h>
#include <forummessage.h>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent = 0 );
    ~MainWindow();
    void updateForumList();
    int getSelectedForum();
signals:
	void subscribeForum();
	void updateClicked();
	void cancelClicked();
	void groupSubscriptions(int forum);
public slots:
	void subscribeForumSlot();
	void groupSubscriptionsSlot();
	void updateClickedSlot();
	void cancelClickedSlot();
	void setForumStatus(int forum, bool reloading);
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
private:
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
    QHash<int, int> forumItems;
    QHash<QListWidgetItem*, ForumGroup> forumGroups;
};

#endif // MAINWINDOW_H
