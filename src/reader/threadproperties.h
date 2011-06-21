#ifndef THREADPROPERTIES_H
#define THREADPROPERTIES_H

#include <QDialog>
#include <siilihai/forumthread.h>
#include <siilihai/forumdatabase.h>

namespace Ui {
    class ThreadProperties;
}

class ThreadProperties : public QDialog {
    Q_OBJECT
public:
    ThreadProperties(QWidget *parent, ForumThread *th, ForumDatabase &fd);
    ~ThreadProperties();
private slots:
    void saveChanges();
    void updateValues();
signals:
    void updateNeeded(ForumSubscription *sub);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::ThreadProperties *ui;
    ForumDatabase &fdb;
    ForumThread *thread;
};

#endif // THREADPROPERTIES_H
