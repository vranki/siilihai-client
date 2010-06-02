#ifndef FORUMPROPERTIES_H
#define FORUMPROPERTIES_H

#include <QDialog>
#include "siilihai/forumdatabase.h"
#include "siilihai/forumsubscription.h"

namespace Ui {
    class ForumProperties;
}

class ForumProperties : public QDialog {
    Q_OBJECT
public:
    ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f);
    ~ForumProperties();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ForumProperties *ui;
    ForumDatabase &fdb;
    ForumSubscription *fs;
};

#endif // FORUMPROPERTIES_H
