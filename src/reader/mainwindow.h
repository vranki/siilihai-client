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

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>
#include <forumgroup.h>
#include <forumthread.h>
#include <forummessage.h>

#include "forumlistwidget.h"
#include "settingsdialog.h"
#include "threadlistwidget.h"
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
	void messageSelected(ForumMessage msg);
	void groupSelected(ForumGroup grp);
private:
	void closeEvent(QCloseEvent *event);
	ForumListWidget *flw;
	ThreadListWidget *tlw;
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
    ForumMessage displayedMessage;
    QSet<int> busyForums;
	QSettings *settings;
	bool readerReady;
};

#endif // MAINWINDOW_H
