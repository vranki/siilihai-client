#ifndef SubscribeWizard_H
#define SubscribeWizard_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QWidget>
#include <QDebug>

#include <siilihai/forumprobe.h>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/parser/forumparser.h>
#include "ui_subscribeforum.h"
#include "ui_subscribeforum_login.h"
#include "ui_subscribeforum_verify.h"

class ForumParser;

class SubscribeWizard: public QWizard {
    Q_OBJECT

public:
    SubscribeWizard(QWidget *parent, SiilihaiProtocol &proto, QSettings &sett);
    ~SubscribeWizard();
    QWizardPage *createIntroPage();
    QWizardPage *createLoginPage();
    QWizardPage *createVerifyPage();
signals:
    void forumAdded(ForumSubscription *fs); //fs valid during signal call

private slots:
    void listParsersFinished(QList <ForumParser*> parsers);
    void updateParserList();
    void pageChanged(int id);
    void wizardAccepted();
    void getParserFinished(ForumParser *parser);
    void comboItemChanged(int id);
    void forumClicked(QListWidgetItem* newItem);
    void newForumAdded(ForumSubscription *sub);
    void probeResults(ForumSubscription *probedSub);
    void forumGot(ForumSubscription *sub);
    void checkUrlValidity();
private:
    SiilihaiProtocol &protocol;
    QSettings &settings;
    Ui::SubscribeForm subscribeForm;
    Ui::SubscribeForumLoginForm subscribeForumLogin;
    Ui::SubscribeForumVerify subscribeForumVerify;
    QList <ForumParser*> allParsers;
    QHash <QListWidgetItem*, ForumParser*> listWidgetItemForum;
    ForumParser *selectedParser;
    ForumParser parser;
    ForumSubscription newForum;
    ForumProbe probe;
};

#endif // SubscribeWizard_H
