#include "threadproperties.h"
#include "ui_threadproperties.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/messageformatting.h>

ThreadProperties::ThreadProperties(QWidget *parent, ForumThread *th, ForumDatabase &fd) :
    QDialog(parent), ui(new Ui::ThreadProperties), fdb(fd), thread(th)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));
    connect(thread, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(thread, SIGNAL(changed(ForumThread*)), this, SLOT(updateValues()));
    updateValues();
}

ThreadProperties::~ThreadProperties()
{
    delete ui;
}

void ThreadProperties::updateValues() {
    QString thrName = thread->name();
    ui->thName->setText(MessageFormatting::stripHtml(thrName));
    ui->thLastchange->setText(thread->lastChange());
    ui->thId->setText(thread->id());
    ui->thMessageCount->setText(QString::number(thread->count()));
    ui->thMessagesPerThread->setValue(thread->getMessagesCount());
    ui->thLastPage->setText(QString::number(thread->lastPage()));
}

void ThreadProperties::saveChanges() {
    int oldMessagesCount = thread->getMessagesCount();

    if(ui->thMessagesPerThread->value() != oldMessagesCount) {
        thread->setGetMessagesCount(ui->thMessagesPerThread->value());
        // If count is LESS than before, delete extra messages!
        if(thread->getMessagesCount() < oldMessagesCount) {
            QList<ForumMessage*> messagesToDelete;
            for(ForumMessage *msg : *thread) {
                if(msg->ordernum() >= thread->getMessagesCount())
                    messagesToDelete.prepend(msg);
            }
            for(ForumMessage *msg : messagesToDelete) {
                thread->removeMessage(msg);
            }
            thread->setHasMoreMessages(true);
        } else { // If count is MORE than before, mark thread&group needing update
            thread->markToBeUpdated();
            thread->group()->markToBeUpdated();
        }
        thread->commitChanges();
        thread->group()->commitChanges();
        emit updateNeeded(thread->group()->subscription());
    }
    deleteLater();
}

void ThreadProperties::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
