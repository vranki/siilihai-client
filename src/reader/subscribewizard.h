#ifndef SubscribeWizard_H
#define SubscribeWizard_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QWidget>
#include <QDebug>

#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumsubscription.h>
#include "ui_subscribeforum.h"
#include "ui_subscribeforum_login.h"
#include "ui_subscribeforum_verify.h"

class SubscribeWizard: public QWizard {
    Q_OBJECT

public:
    SubscribeWizard(QWidget *parent, SiilihaiProtocol &proto, QString &baseUrl, QSettings &sett);
    ~SubscribeWizard();
    QWizardPage *createIntroPage();
    QWizardPage *createLoginPage();
    QWizardPage *createVerifyPage();
signals:
    void forumAdded(ForumSubscription *fs);

private slots:
    void listParsersFinished(QList <ForumParser*> parsers);
    void updateParserList();
    void pageChanged(int id);
    void wizardAccepted();
    void getParserFinished(ForumParser *parser);
    void comboItemChanged(int id);
    void forumClicked(QListWidgetItem* newItem);
private:
    QWizard wizard;
    SiilihaiProtocol &protocol;
    QSettings &settings;
    QProgressDialog *progress;
    Ui::SubscribeForm subscribeForm;
    Ui::SubscribeForumLoginForm subscribeForumLogin;
    Ui::SubscribeForumVerify subscribeForumVerify;
    QList <ForumParser*> allParsers;
    QHash <QListWidgetItem*, ForumParser*> listWidgetItemForum;
    ForumParser *selectedParser;
    ForumParser *parser;
};

#endif // SubscribeWizard_H
