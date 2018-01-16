#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H
// Qt5
#include <QWidget>
#include <QDesktopServices>
#include <QTimer>
#include <siilihai/parser/forumparser.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/parser/patternmatcher.h>
#include "ui_patterneditor.h"

class ForumSubscription;
class PatternMatcher;

class PatternEditor : public QWidget
{
    Q_OBJECT

public:
    PatternEditor(ParserEngine &eng, ForumParser *par, ForumSubscription *fos, QWidget *parent = 0);
    ~PatternEditor();
    QString pattern();
    void setPattern(QString txt);
    virtual QString tabName();
    virtual QIcon tabIcon();

public slots:
    virtual void parserUpdated();
    virtual void reset()=0;

private slots:
    virtual void downloadList() = 0;
    virtual void testPageSpanning() = 0;
    virtual void patternChanged() = 0;
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
    QTextCursor patternEditorCursor;
    ParserEngine &engine;
    ForumSubscription *subscription, *downloadSubscription;
    ForumParser *parser, downloadParser;
    QTimer editTimeout;
    PatternMatcher *matcher;
    bool pageSpanningTest;
};

#endif // PATTERNEDITOR_H
