#ifndef PARSERMAKER_H
#define PARSERMAKER_H

#include <QtGui/QMainWindow>
#include <QtGui>

#include <siilihai/parserdatabase.h>
#include <siilihai/siilihaiprotocol.h>
#include <siilihai/forumparser.h>
#include <siilihai/forumsession.h>
#include <siilihai/forumgroup.h>
#include <siilihai/forumthread.h>

#include "downloaddialog.h"
#include "openrequestdialog.h"
#include "patterneditor.h"
#include "threadlistpatterneditor.h"
#include "grouplistpatterneditor.h"
#include "messagelistpatterneditor.h"

#include "ui_parsermaker.h"

class ParserMaker : public QMainWindow
{
    Q_OBJECT

public:
    ParserMaker(QWidget *parent, ParserDatabase &pd, QSettings &s, SiilihaiProtocol &p);
    ~ParserMaker();

public slots:
	void updateState();
	void openClicked();
	void newFromRequestClicked();
	void saveClicked();
	void saveAsNewClicked();
	void testForumUrlClicked();
	void parserLoaded(ForumParser p);
	void saveParserFinished(int newId, QString message);
	void requestSelected(ForumRequest req);
	void tryLogin();
	void tryWithoutLogin();
	void loginFinished(bool success);
	void networkFailure(QString txt);
	void helpClicked();
signals:
	void parserSaved(ForumParser savedParser);

private:
	void closeEvent(QCloseEvent *event);
	Ui::ParserMakerWindow ui;
    ParserDatabase &pdb;
	QSettings &settings;
	SiilihaiProtocol &protocol;
	ForumParser parser;
	ForumSession session;
	ForumSubscription subscription;
	PatternEditor *groupListEditor, *threadListEditor, *messageListEditor;
	ForumGroup selectedGroup;
	ForumThread selectedThread;
	bool loginWithoutCredentials;
};

#endif // PARSERMAKER_H
