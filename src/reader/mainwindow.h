#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QList>
#include <QSet>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QPushButton>

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>
#include <forumgroup.h>
#include <forumthread.h>
#include <forummessage.h>

#include "forumlistwidget.h"

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ParserDatabase &pd, ForumDatabase &fd, QSettings *s, QWidget *parent = 0 );
    ~MainWindow();
    ForumListWidget* forumList();
signals:
	void subscribeForum();
	void unsubscribeForum(int forumid);
	void updateClicked();
	void reportClicked(int forumid);
	void updateClicked(int forumid, bool force);
	void cancelClicked();
	void groupSubscriptions(int forum);
	void messageRead(ForumMessage message);
	void launchParserMaker();
public slots:
	void subscribeForumSlot();
	void unsubscribeForumSlot();
	void groupSubscriptionsSlot();
	void updateClickedSlot();
	void updateSelectedClickedSlot();
	void forceUpdateSelectedClickedSlot();
	void cancelClickedSlot();
	void viewInBrowserClickedSlot();
	void hideClickedSlot();
	void reportClickedSlot();
	void groupSelected(ForumGroup fg);
	void messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev);
	void setForumStatus(int forum, bool reloading, float progress);
	void launchParserMakerSlot();
	void setReaderReady(bool ready);
private:
	void updateMessageRead(QTreeWidgetItem *item);
	void closeEvent(QCloseEvent *event);
	ForumListWidget *flw;
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
    ForumMessage displayedMessage;
    QHash<QTreeWidgetItem*, ForumMessage> forumMessages;
    QSet<int> busyForums;
	QSettings *settings;
	bool readerReady;
};

#endif // MAINWINDOW_H
