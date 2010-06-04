#include "forumproperties.h"
#include "ui_forumproperties.h"

ForumProperties::ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f, ParserDatabase &p) :
    QDialog(parent),
    ui(new Ui::ForumProperties), fdb(f), pdb(p)
{
    ui->setupUi(this);
    fs = s;
    ui->forumName->setText(fs->alias());
    ui->threads_per_group->setValue(fs->latest_threads());
    ui->messages_per_thread->setValue(fs->latest_messages());
    if(pdb.getParser(fs->parser()).supportsLogin()) {
        ui->authenticationGroupbox->setEnabled(true);
        if(fs->username().length()>0)  {
            ui->authenticationGroupbox->setChecked(true);
            ui->username->setText(fs->username());
            ui->password->setText(fs->password());
        }
    } else {
        ui->authenticationGroupbox->setEnabled(false);
    }
    connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
}

ForumProperties::~ForumProperties()
{
    delete ui;
}

void ForumProperties::changeEvent(QEvent *e)
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

void ForumProperties::saveChanges() {
    bool update = false;
    fs->setAlias(ui->forumName->text());
    if(fs->latest_threads() != ui->threads_per_group->value()) {
        fs->setLatestThreads(ui->threads_per_group->value());
        foreach(ForumGroup *grp, fdb.listGroups(fs)) {
            grp->setLastchange("UPDATE_NEEDED");
            fdb.updateGroup(grp);
        }
        update = true;
    }

    if(fs->latest_messages() != ui->messages_per_thread->value()) {
        foreach(ForumGroup *grp, fdb.listGroups(fs)) {
            foreach(ForumThread *thread, fdb.listThreads(grp)) {
                if(thread->getMessagesCount() != ui->messages_per_thread->value()) {
                    thread->setGetMessagesCount(ui->messages_per_thread->value());
                    thread->setLastchange("UPDATE_NEEDED");
                    fdb.updateThread(thread);
                    grp->setLastchange("UPDATE_NEEDED");
                    fdb.updateGroup(grp);
                    update = true;
                }
            }
        }
    }

    if(ui->authenticationGroupbox->isEnabled() && ui->username->text().length() > 0) {
        fs->setUsername(ui->username->text());
        fs->setPassword(ui->password->text());
    } else {
        fs->setUsername(QString::null);
        fs->setPassword(QString::null);
    }
    fdb.updateSubscription(fs);
    if(update)
        emit updateNeeded(fs);
}
