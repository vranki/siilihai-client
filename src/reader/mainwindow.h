#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QList>
#include <QSet>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>

#include <siilihai/forumdatabase/forumdatabase.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/siilihaisettings.h>

#ifdef INCLUDE_SIILIHAI_VERSION
#include "../../siilihai-version.h"
#endif
#include "messageviewwidget.h"
#include "ui_mainwindow.h"

class ParserEngine;
class ForumListWidget;
class ThreadListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ForumDatabase &fd, SiilihaiSettings *s, QWidget *parent = 0 );
    ~MainWindow();
signals:
    void subscribeForum();
    void unsubscribeForum(ForumSubscription *sub);
    void updateClicked();
    void updateThread(ForumThread*, bool);
    void reportClicked(ForumSubscription *sub);
    void updateClicked(ForumSubscription *sub, bool force);
    void cancelClicked();
    void groupSubscriptions(ForumSubscription *sub);
    void unsubscribeGroup(ForumGroup *group);
    void launchParserMaker();
    void offlineModeSet(bool ol);
    void haltRequest();
    void settingsChanged(bool byUser);
    void moreMessagesRequested(ForumThread* thread);
    void forumUpdateNeeded(ForumSubscription *sub);
    void unregisterSiilihai();
    void showStatusMessage(QString message);
    void groupUnselected(ForumGroup *group); // and could now be synced etc
    void startSyncClicked();
    void endSyncClicked();
    void updateAllParsers();

public slots:
    void updateEngineStateChanged(UpdateEngine *engine, UpdateEngine::UpdateEngineState newState,
                                  UpdateEngine::UpdateEngineState oldState);
    void setOffline(bool offline);
    void showMessage(QString msg, int time=5000);

private slots:
    void unsubscribeForumSlot();
    void groupSubscriptionsSlot();
    void updateSelectedClickedSlot();
    void forceUpdateSelectedClickedSlot();
    void viewInBrowserClickedSlot();
    void hideClickedSlot();
    void reportClickedSlot();
    void offlineClickedSlot();
    void markForumRead(bool read=true);
    void markForumUnread();
    void markGroupRead(bool read=true);
    void markGroupUnread();
    void forumPropertiesSlot();
    void threadPropertiesSlot(ForumThread *thread);
    void about();
    void settingsDialog();
    void settingsDialogAccepted();
    void userAccountSettings();
    void updateEnabledButtons();
    void subscriptionFound(ForumSubscription *sub);
    void newThreadClickedSlot();
    void replyClickedSlot();
    void replyQuotedClickedSlot();
protected:
    bool eventFilter(QObject *object, QEvent *event);
private:
    void closeEvent(QCloseEvent *event);
    ForumListWidget *flw;
    ThreadListWidget *tlw;
    MessageViewWidget *mvw;
    Ui::MainWindowClass ui;
    ForumDatabase &fdb;
    SiilihaiSettings *settings;
    bool offline;
};

#endif // MAINWINDOW_H
