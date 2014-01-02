#include "composemessage.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/updateengine.h>
#include <siilihai/messageformatting.h>

ComposeMessage::ComposeMessage(QWidget *parent, SiilihaiSettings &shs) :
    QDialog(parent), group(0), thread(0), settings(shs)
{
    ui.setupUi(this);
    ui.bodyEdit->appendPlainText("--\nSent using Siilihai web forum reader");
    ui.bodyEdit->moveCursor(QTextCursor::Start);
    connect(ui.sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));
}

void ComposeMessage::newThreadIn(ForumGroup *grp) {
    Q_ASSERT(grp);
    group = grp;
    ui.toLabel->setText(grp->subscription()->alias() + " / " + grp->name());
    ui.bodyEdit->setPlainText(settings.signature());
    connect(grp, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(grp->subscription()->updateEngine(), SIGNAL(messagePosted(ForumSubscription*)), this, SLOT(deleteLater()));
}

void ComposeMessage::newReplyTo(ForumThread *thr) {
    connect(thr, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    thread = thr;
    group = thr->group();
    ui.toLabel->setText(group->subscription()->alias() + " / " + group->name());
    QString subject = thr->name();
    if(!subject.startsWith("Re:")) subject = "Re: " + subject;
    ui.subjectLine->setText(subject);
    ui.bodyEdit->setPlainText(settings.signature());
    connect(group->subscription()->updateEngine(), SIGNAL(messagePosted(ForumSubscription*)), this, SLOT(deleteLater()));
}

void ComposeMessage::newReplyTo(ForumMessage *msg) {
    connect(msg, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    newReplyTo(msg->thread());
    QString body = msg->body();
    body = MessageFormatting::stripHtml(body);
    body = "[quote]\n" + body + "\n[/quote]\n\n"; // @todo won't probably work
    body += settings.signature();
    ui.bodyEdit->setPlainText(body);
}

void ComposeMessage::sendClicked() {
    group->subscription()->updateEngine()->postMessage(group, thread, ui.subjectLine->text(), ui.bodyEdit->toPlainText());
}
