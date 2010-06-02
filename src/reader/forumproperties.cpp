#include "forumproperties.h"
#include "ui_forumproperties.h"

ForumProperties::ForumProperties(QWidget *parent, ForumSubscription *s, ForumDatabase &f) :
    QDialog(parent),
    ui(new Ui::ForumProperties), fdb(f)
{
    ui->setupUi(this);
    fs = s;
    ui->forumName->setText("");
    ui->forumUrl->setText("");
    ui->threads_per_group->setValue(fs->latest_threads());
    ui->messages_per_thread->setValue(fs->latest_messages());
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
