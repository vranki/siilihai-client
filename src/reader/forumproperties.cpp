#include "forumproperties.h"
#include "ui_forumproperties.h"
#include "siilihai/forumgroup.h"
#include "siilihai/forumthread.h"

ForumProperties::ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f, ParserDatabase &p) :
    QDialog(parent),
    ui(new Ui::ForumProperties), fdb(f), pdb(p)
{
    ui->setupUi(this);
    fs = s;
    connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));
    connect(fs, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(fs, SIGNAL(changed(ForumSubscription*)), this, SLOT(updateValues()));
    updateValues();
}

ForumProperties::~ForumProperties()
{
    delete ui;
}

void ForumProperties::updateValues() {
    ui->forumName->setText(fs->alias());
    ui->threads_per_group->setValue(fs->latestThreads());
    ui->messages_per_thread->setValue(fs->latestMessages());
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
    if(fs->latestThreads() != ui->threads_per_group->value()) {
        fs->setLatestThreads(ui->threads_per_group->value());
        foreach(ForumGroup *group, fs->values()) {
            group->markToBeUpdated();
            group->commitChanges();
        }
        update = true;
    }

    if(fs->latestMessages() != ui->messages_per_thread->value()) {
        foreach(ForumGroup *grp, fs->values()) {
            foreach(ForumThread *thread, grp->values()) {
                if(thread->getMessagesCount() != ui->messages_per_thread->value()) {
                    thread->setGetMessagesCount(ui->messages_per_thread->value());
                    thread->markToBeUpdated();
                    thread->commitChanges();
                    grp->markToBeUpdated();
                    grp->commitChanges();
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
    if(update)
        emit forumUpdateNeeded(fs);
    deleteLater();
}
