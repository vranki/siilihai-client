#ifndef FORUMPROPERTIES_H
#define FORUMPROPERTIES_H

#include <QDialog>

class ForumSubscription;
#include <siilihai/forumdatabase.h>

namespace Ui {
    class ForumProperties;
}

class ForumProperties : public QDialog {
    Q_OBJECT
public:
    ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f);
    ~ForumProperties();
signals:
    void forumUpdateNeeded(ForumSubscription *sub);

private slots:
    void saveChanges();
    void updateValues();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::ForumProperties *ui;
    ForumDatabase &fdb;
    ForumSubscription *fs;
};

#endif // FORUMPROPERTIES_H
