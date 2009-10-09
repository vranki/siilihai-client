#ifndef SubscribeWizard_H
#define SubscribeWizard_H

#include <QtGui>
#include <QObject>
#include <QWizard>
#include <QWidget>
#include <QDebug>

#include <siilihaiprotocol.h>
#include <forumsubscription.h>
#include "ui_subscribeforum.h"
#include "ui_subscribeforum_login.h"
#include "ui_subscribeforum_verify.h"

class SubscribeWizard: public QWizard {
	Q_OBJECT

public:
	SubscribeWizard(QWidget *parent, SiilihaiProtocol &proto, QString &baseUrl);
	~SubscribeWizard();
	QWizardPage *createIntroPage();
	QWizardPage *createLoginPage();
	QWizardPage *createVerifyPage();
	//int nextId() const;
signals:
	void forumAdded(ForumParser fp, ForumSubscription fs);

public slots:
	void listParsersFinished(QList <ForumParser> parsers);
	void updateParserList();
	void pageChanged(int id);
	void wizardAccepted();
	void getParserFinished(ForumParser parser);

private:
	QWizard wizard;
	SiilihaiProtocol &protocol;
	QProgressDialog *progress;
	QSettings settings;
    Ui::SubscribeForm subscribeForm;
    Ui::SubscribeForumLoginForm subscribeForumLogin;
    Ui::SubscribeForumVerify subscribeForumVerify;
    QList <ForumParser> allParsers;
    QHash <QListWidgetItem*, ForumParser*> listWidgetItemForum;
    ForumParser *selectedParser;
	ForumParser parser;
};

#endif // SubscribeWizard_H
