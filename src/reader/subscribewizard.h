#ifndef SubscribeWizard_H
#define SubscribeWizard_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QWidget>
#include <QDebug>

#include <siilihai/forumprobe.h>
#include <siilihai/subscriptionmanagement.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/parser/forumparser.h>
#include <siilihai/siilihaisettings.h>
#include "ui_subscribeforum.h"
#include "ui_subscribeforum_login.h"
#include "ui_subscribeforum_verify.h"

class ForumParser;

class SubscribeWizard: public QWizard {
    Q_OBJECT

public:
    SubscribeWizard(QWidget *parent, SubscriptionManagement *subscriptionManagement);
    ~SubscribeWizard();
    QWizardPage *createIntroPage();
    QWizardPage *createLoginPage();
    QWizardPage *createVerifyPage();

signals:
    void forumAdded(ForumSubscription *fs); // fs valid during signal call

private slots:
    void forumListChanged();
    void updateForumList();
    void newForumChanged(ForumSubscription* sub);
    void pageChanged(int id);
    void wizardAccepted();
    void comboItemChanged(int id);
    void forumClicked(QListWidgetItem* newItem);
    void checkUrlValidity();
private:
    SubscriptionManagement *m_subscriptionManagement;
    Ui::SubscribeForm subscribeForm;
    Ui::SubscribeForumLoginForm subscribeForumLogin;
    Ui::SubscribeForumVerify subscribeForumVerify;
    QHash <QListWidgetItem*, ForumSubscription*> listWidgetItemForum;
    ForumSubscription *selectedForum;
};

#endif // SubscribeWizard_H
