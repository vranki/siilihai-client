#ifndef PARSERMAKER_H
#define PARSERMAKER_H

#include <QtGui/QMainWindow>
#include <QtGui>
#include <QSqlDatabase>

#include <parserdatabase.h>
#include <siilihaiprotocol.h>
#include <forumparser.h>
#include <forumsession.h>
#include <forumgroup.h>
#include <forumthread.h>

#include "../commondefs.h"
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
    ParserMaker(QWidget *parent = 0);
    ~ParserMaker();

public slots:
	void updateState();
	void loginFinished(bool success, QString motd);
	void openClicked();
	void newFromRequestClicked();
	void saveClicked();
	void saveAsNewClicked();
	void testForumUrlClicked();
	void parserLoaded(ForumParser p);
	void saveParserFinished(int newId, QString message);
	void requestSelected(ForumRequest req);
private:
	Ui::ParserMakerWindow ui;
    ParserDatabase pdb;
    QSqlDatabase db;
	QSettings settings;
	SiilihaiProtocol protocol;
	ForumParser parser;
	ForumSession session;
	ForumSubscription subscription;
	PatternEditor *groupListEditor, *threadListEditor, *messageListEditor;
	ForumGroup selectedGroup;
	ForumThread selectedThread;
};

#endif // PARSERMAKER_H
