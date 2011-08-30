#ifndef GROUPLISTPATTERNEDITOR_H_
#define GROUPLISTPATTERNEDITOR_H_
#include <siilihai/forumgroup.h>

#include "patterneditor.h"

class GroupListPatternEditor : public PatternEditor {
    Q_OBJECT

public:
    GroupListPatternEditor(ForumSession &ses, ForumParser *par, ForumSubscription *fos, QWidget *parent = 0);
    virtual ~GroupListPatternEditor();
    virtual QIcon tabIcon();
    virtual QString tabName();

public slots:
    virtual void listGroupsFinished(QList<ForumGroup*> &groups);
    virtual void downloadList();
    virtual void testPageSpanning();
    void resultCellActivated(int row, int column);
    virtual void parserUpdated();
    virtual void patternChanged();

signals:
    void groupSelected(ForumGroup *grp);
private:
    QHash<int, ForumGroup*> listGroups;
};

#endif /* GROUPLISTPATTERNEDITOR_H_ */
