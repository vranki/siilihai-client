#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QList>
#include <QSet>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QPushButton>
#include <QMessageBox>

#include <siilihai/parserdatabase.h>
#include <siilihai/forumdatabase.h>
#include <siilihai/forumsubscription.h>
#include <siilihai/forumgroup.h>
#include <siilihai/forumthread.h>
#include <siilihai/forummessage.h>

#include "forumlistwidget.h"
#include "settingsdialog.h"
#include "threadlistwidget.h"
#include "messageviewwidget.h"
#include "messageformatting.h"

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ParserDatabase &pd, ForumDatabase &fd, QSettings *s, QWidget *parent = 0 );
    ~MainWindow();
    ForumListWidget* forumList();
    ThreadListWidget* threadList();
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
	void offlineModeSet(bool ol);
	void haltRequest();
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
	void offlineClickedSlot();
	void markForumRead(bool read=true);
	void markForumUnread();
	void markGroupRead(bool read=true);
	void markGroupUnread();
	void setForumStatus(int forum, bool reloading, float progress);
	void launchParserMakerSlot();
	void setReaderReady(bool ready, bool offline);
private slots:
	void about();
	void settingsDialog();
	void groupSelected(ForumGroup grp);
	void messageSelected(const ForumMessage &msg);
	void updateEnabled();
private:
	void closeEvent(QCloseEvent *event);

	ForumListWidget *flw;
	ThreadListWidget *tlw;
	MessageViewWidget *mvw;
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
    QSet<int> busyForums;
	QSettings *settings;
	bool readerReady, offline;
};

#endif // MAINWINDOW_H
