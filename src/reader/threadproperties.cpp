#include "threadproperties.h"
#include "ui_threadproperties.h"

ThreadProperties::ThreadProperties(QWidget *parent, ForumThread *th, ForumDatabase &fd) :
    QDialog(parent),
    ui(new Ui::ThreadProperties), fdb(fd), thread(th)
{
    ui->setupUi(this);
    ui->thName->setText(thread->name());
    ui->thLastchange->setText(thread->lastchange());
    ui->thId->setText(thread->id());
    ui->thMessageCount->setText(QString::number(thread->messages().count()));
    ui->thMessagesPerThread->setValue(thread->getMessagesCount());
    connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

ThreadProperties::~ThreadProperties()
{
    delete ui;
}

void ThreadProperties::saveChanges() {
    int oldMessagesCount = thread->getMessagesCount();

    if(ui->thMessagesPerThread->value() != oldMessagesCount) {
        thread->setGetMessagesCount(ui->thMessagesPerThread->value());
        // If count is LESS than before, delete extra messages!
        if(thread->getMessagesCount() < oldMessagesCount) {
            QList<ForumMessage*> messagesToDelete;
            foreach(ForumMessage *msg, thread->messages()) {
                if(msg->ordernum() >= thread->getMessagesCount())
                    messagesToDelete.prepend(msg);
            }
            foreach(ForumMessage *msg, messagesToDelete) {
                fdb.deleteMessage(msg);
            }
            thread->setHasMoreMessages(true);
        } else { // If count is MORE than before, mark thread&group needing update
            thread->setLastchange("UPDATE_NEEDED");
            thread->group()->setLastchange("UPDATE_NEEDED");
        }
        thread->commitChanges();
        thread->group()->commitChanges();
        emit updateNeeded(thread->group()->subscription());
    }
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
