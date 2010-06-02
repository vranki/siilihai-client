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
#include "forumproperties.h"

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
    void unsubscribeForum(ForumSubscription *sub);
    void updateClicked();
    void reportClicked(ForumSubscription *sub);
    void updateClicked(ForumSubscription *sub, bool force);
    void cancelClicked();
    void groupSubscriptions(ForumSubscription *sub);
    void unsubscribeGroup(ForumGroup *group);
    void messageRead(ForumMessage message);
    void launchParserMaker();
    void offlineModeSet(bool ol);
    void haltRequest();
    void settingsChanged(bool byUser);
    void moreMessagesRequested(ForumThread* thread);
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
    void setForumStatus(ForumSubscription *sub, bool reloading, float progress);
    void launchParserMakerSlot();
    void setReaderReady(bool ready, bool offline);
    void forumSelected(ForumSubscription *sub);
    void groupSelected(ForumGroup *grp);
    void forumPropertiesSlot();
private slots:
    void about();
    void settingsDialog();
    void settingsDialogAccepted();
    void messageSelected(ForumMessage *msg);
    void updateEnabledButtons();

private:
    void closeEvent(QCloseEvent *event);

    ForumListWidget *flw;
    ThreadListWidget *tlw;
    MessageViewWidget *mvw;
    Ui::MainWindowClass ui;
    ForumDatabase &fdb;
    ParserDatabase &pdb;
    QSet<ForumSubscription*> busyForums;
    QSettings *settings;
    bool readerReady, offline;
    QActionGroup viewAsGroup;
};

#endif // MAINWINDOW_H
