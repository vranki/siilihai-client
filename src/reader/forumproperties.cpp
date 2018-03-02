#include "forumproperties.h"
#include "ui_forumproperties.h"
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/parser/parserdatabase.h>

ForumProperties::ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f) :
    QDialog(parent), ui(new Ui::ForumProperties), fdb(f) {
    ui->setupUi(this);
    fs = s;
    connect(this, SIGNAL(accepted()), this, SLOT(saveChanges()));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));
    connect(fs, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(fs, SIGNAL(changed()), this, SLOT(updateValues()));
    updateValues();
}

ForumProperties::~ForumProperties() {
    delete ui;
}

void ForumProperties::updateValues() {
    ui->forumName->setText(fs->alias());
    ui->threads_per_group->setValue(fs->latestThreads());
    ui->messages_per_thread->setValue(fs->latestMessages());
    bool supportsLogin = true;
    if(fs->provider() == ForumSubscription::FP_PARSER) {
        supportsLogin = qobject_cast<ForumSubscriptionParsed*>(fs)->parserEngine()->parser()->supportsLogin();
    }
    if(supportsLogin) {
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

void ForumProperties::changeEvent(QEvent *e) {
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
    disconnect(fs, SIGNAL(changed()), this, SLOT(updateValues()));

    fs->setAlias(ui->forumName->text());
    if(fs->latestThreads() != ui->threads_per_group->value()) {
        fs->setLatestThreads(ui->threads_per_group->value());
        for(ForumGroup *group : *fs) {
            group->markToBeUpdated();
            group->commitChanges();
        }
    }
    if(fs->latestMessages() != ui->messages_per_thread->value()) {
        for(ForumGroup *grp : *fs) {
            for(ForumThread *thread : *grp) {
                if(thread->getMessagesCount() != ui->messages_per_thread->value()) {
                    thread->setGetMessagesCount(ui->messages_per_thread->value());
                    thread->markToBeUpdated();
                    thread->commitChanges();
                    grp->markToBeUpdated();
                    grp->commitChanges();
                }
            }
        }
    }
    if(ui->authenticationGroupbox->isChecked() && ui->username->text().length() > 0) {
        fs->setUsername(ui->username->text());
        fs->setPassword(ui->password->text());
        fs->setAuthenticated(true);
    } else {
        fs->setUsername(QString::null);
        fs->setPassword(QString::null);
        fs->setAuthenticated(false);
    }
    emit forumUpdateNeeded(fs);
    deleteLater();
}
