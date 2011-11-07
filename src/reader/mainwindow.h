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
#include <QSettings>

#include <siilihai/forumdatabase.h>
#include <siilihai/parserengine.h>

#ifdef INCLUDE_SIILIHAI_VERSION
#include "../../siilihai-version.h"
#endif

#include "ui_mainwindow.h"

class ParserEngine;
class ForumListWidget;
class ThreadListWidget;
class MessageViewWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ForumDatabase &fd, QSettings *s, QWidget *parent = 0 );
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
    void launchParserMaker();
    void offlineModeSet(bool ol);
    void haltRequest();
    void settingsChanged(bool byUser);
    void moreMessagesRequested(ForumThread* thread);
    void forumUpdateNeeded(ForumSubscription *sub);
    void unregisterSiilihai();
public slots:
    void parserEngineStateChanged(ParserEngine *engine, ParserEngine::ParserEngineState newState,
                                  ParserEngine::ParserEngineState oldState);
    void setReaderReady(bool ready, bool offline);
    void syncProgress(float progress, QString message);
private slots:
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
    void launchParserMakerSlot();
    void forumSelected(ForumSubscription *sub);
    void groupSelected(ForumGroup *grp);
    void forumPropertiesSlot();
    void threadPropertiesSlot(ForumThread *thread);
    void about();
    void settingsDialog();
    void settingsDialogAccepted();
    void userAccountSettings();
    void messageSelected(ForumMessage *msg);
    void updateEnabledButtons();
    void subscriptionFound(ForumSubscription *sub);
protected:
    bool eventFilter(QObject *object, QEvent *event);
private:
    void closeEvent(QCloseEvent *event);
    ForumListWidget *flw;
    ThreadListWidget *tlw;
    MessageViewWidget *mvw;
    Ui::MainWindowClass ui;
    ForumDatabase &fdb;
    QSet<ParserEngine*> busyParserEngines;
    QSettings *settings;
    bool readerReady, offline;
    QActionGroup viewAsGroup;
};

#endif // MAINWINDOW_H
