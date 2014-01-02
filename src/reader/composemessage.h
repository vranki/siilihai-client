#ifndef COMPOSEMESSAGE_H
#define COMPOSEMESSAGE_H

#include <QWidget>
#include "ui_composemessage.h"
#include <siilihai/siilihaisettings.h>

class ForumGroup;
class ForumThread;
class ForumMessage;

class ComposeMessage : public QDialog
{
    Q_OBJECT
public:
    explicit ComposeMessage(QWidget *parent, SiilihaiSettings &shs);
    void newThreadIn(ForumGroup *grp);
    void newReplyTo(ForumThread *thr);
    void newReplyTo(ForumMessage *msg);

private slots:
    void sendClicked();

private:
    Ui::ComposeMessageDialog ui;
    ForumGroup *group;
    ForumThread *thread;
    SiilihaiSettings &settings;
};

#endif // COMPOSEMESSAGE_H
