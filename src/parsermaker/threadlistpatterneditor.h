#ifndef THREADLISTPATTERNEDITOR_H_
#define THREADLISTPATTERNEDITOR_H_

#include "patterneditor.h"

class ForumGroup;

class ThreadListPatternEditor : public PatternEditor {
    Q_OBJECT

public:
    ThreadListPatternEditor(ParserEngine &eng, ForumParser *par, ForumSubscription *fos, QWidget *parent = 0);
    virtual ~ThreadListPatternEditor();
    virtual QString tabName();
    virtual QIcon tabIcon();

public slots:
    virtual void downloadList();
    virtual void testPageSpanning();

    void setGroup(ForumGroup *grp);
    void resultCellActivated(int row, int column);
    virtual void parserUpdated();
    virtual void listThreadsFinished(QList<ForumThread*>& threads, ForumGroup *group);
    virtual void patternChanged();
    virtual void reset();
signals:
    void threadSelected(ForumThread *thread);
private:
    ForumGroup *currentGroup;
    QHash<int, ForumThread*> listThreads; // Contains local copies of threads
};

#endif /* THREADLISTPATTERNEDITOR_H_ */
