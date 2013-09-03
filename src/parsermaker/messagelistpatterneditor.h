#ifndef MESSAGELISTPATTERNEDITOR_H_
#define MESSAGELISTPATTERNEDITOR_H_
#include <QMessageBox>

#include "patterneditor.h"

class ForumParser;
class ForumSubscription;

class MessageListPatternEditor: public PatternEditor {
    Q_OBJECT

public:
    MessageListPatternEditor(ParserEngine &eng, ForumParser *par,
                             ForumSubscription *fos, QWidget *parent = 0);
    virtual ~MessageListPatternEditor();
    virtual QString tabName();
    virtual QIcon tabIcon();

public slots:
    virtual void downloadList();
    virtual void testPageSpanning();

    void setThread(ForumThread *thread);
    void resultCellActivated(int row, int column);
    virtual void parserUpdated();
    virtual void listMessagesFinished(QList<ForumMessage*> &messages,
                                      ForumThread *thread, bool more);
    virtual void patternChanged();

signals:

private:
    ForumThread *currentThread;
    QHash<int, QString> bodies; // List row/body
};

#endif /* MESSAGELISTPATTERNEDITOR_H_ */
