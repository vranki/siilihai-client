#ifndef PARSERMAKER_H
#define PARSERMAKER_H

#include <QtGui/QMainWindow>
#include <QtGui>
#include <QSqlDatabase>

#include <parserdatabase.h>
#include <siilihaiprotocol.h>
#include <forumparser.h>
#include <forumsession.h>

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
private:
	Ui::ParserMakerWindow ui;
    ParserDatabase pdb;
    QSqlDatabase db;
	QSettings settings;
	SiilihaiProtocol protocol;
	ForumParser parser;
	ForumSession session;
};

#endif // PARSERMAKER_H
