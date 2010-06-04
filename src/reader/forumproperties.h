#ifndef FORUMPROPERTIES_H
#define FORUMPROPERTIES_H

#include <QDialog>
#include "siilihai/forumdatabase.h"
#include "siilihai/forumsubscription.h"
#include "siilihai/parserdatabase.h"

namespace Ui {
    class ForumProperties;
}

class ForumProperties : public QDialog {
    Q_OBJECT
public:
    ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f, ParserDatabase &p);
    ~ForumProperties();
signals:
    void updateNeeded(ForumSubscription *sub);

private slots:
    void saveChanges();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ForumProperties *ui;
    ForumDatabase &fdb;
    ParserDatabase &pdb;
    ForumSubscription *fs;
};

#endif // FORUMPROPERTIES_H
