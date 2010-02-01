#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

#include <QtGui/QWidget>
#include <QDesktopServices>
#include <QTimer>
#include <siilihai/patternmatcher.h>
#include <siilihai/forumparser.h>
#include <siilihai/forumsession.h>

#include "ui_patterneditor.h"

class PatternEditor : public QWidget
{
    Q_OBJECT

public:
    PatternEditor(ForumSession &ses, ForumParser &par, ForumSubscription *fos, QWidget *parent = 0);
    ~PatternEditor();
    QString pattern();
    void setPattern(QString txt);
    virtual QString tabName();
    virtual QIcon tabIcon();

public slots:
	virtual void parserUpdated();
	virtual void downloadList() = 0;
	virtual void testPageSpanning() = 0;
	virtual void patternChanged() = 0;
	virtual void listGroupsFinished(QList<ForumGroup*> groups);
	virtual void listThreadsFinished(QList<ForumThread*> threads, ForumGroup *group);
	virtual void listMessagesFinished(QList<ForumMessage*> messages, ForumThread *thread);
	virtual void resultCellActivated(int row, int column)=0;

	void viewInBrowser();
	void dataMatchingStart(QString &html);
	void dataMatchingEnd();
	void dataMatched(int pos, QString data, PatternMatchType type);

private slots:
	void textEdited(); // Called instantaneously
	void updateCount();

protected:

	Ui::PatternEditorClass ui;
	QTextCursor groupListCursor;
	ForumSession &session;
	ForumParser &parser, downloadParser;
	PatternMatcher *matcher;
	ForumSubscription *subscription, *downloadSubscription;
	QHash<int, QString> listIds;
	bool pageSpanningTest;
	QTimer editTimeout;
};

#endif // PATTERNEDITOR_H
