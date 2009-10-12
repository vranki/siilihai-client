#ifndef PARSERMAKER_H
#define PARSERMAKER_H

#include <QtGui/QMainWindow>
#include <QtGui>
#include <QSqlDatabase>

#include <parserdatabase.h>
#include <siilihaiprotocol.h>
#include <forumparser.h>
#include <forumsession.h>
#include <patternmatcher.h>

#include "../commondefs.h"
#include "downloaddialog.h"

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
	void saveClicked();
	void saveAsNewClicked();
	void parserLoaded(ForumParser p);
	void saveParserFinished(int newId, QString message);
	void downloadGroupList();
	void groupListPatternChanged(QString txt);

	void dataMatchingStart(QString &html);
	void dataMatchingEnd();
	void dataMatched(int pos, QString data, PatternMatchType type);

	void listGroupsFinished(QList<ForumGroup> groups);
	void listThreadsFinished(QList<ForumThread> threads, ForumGroup group);
	void listMessagesFinished(QList<ForumMessage> messages, ForumThread thread);

private:
	Ui::ParserMakerWindow ui;
    ParserDatabase pdb;
    QSqlDatabase db;
	QSettings settings;
	SiilihaiProtocol protocol;
	ForumParser parser;
	ForumSession session;
	PatternMatcher matcher;
	ForumSubscription subscription;
	QTextCursor groupListCursor;
	bool groupListDownloaded;
};

#endif // PARSERMAKER_H
