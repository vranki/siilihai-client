#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, SiilihaiSettings *s)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
    settings = s;
    if(settings->noAccount())
        ui.enableSyncing->setEnabled(false);
    ui.enableSyncing->setChecked(settings->syncEnabled());
    ui.httpProxy->setText(settings->httpProxy());
    ui.threads_per_group->setValue(settings->threadsPerGroup());
    ui.messages_per_thread->setValue(settings->messagesPerThread());
    ui.show_more_count->setValue(settings->showMoreCount());
    ui.signatureEdit->setPlainText(settings->signature());
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::closeClicked() {
    settings->setValue("preferences/sync_enabled", ui.enableSyncing->isChecked());
    settings->setValue("preferences/http_proxy", ui.httpProxy->text());
    settings->setValue("preferences/threads_per_group", QString::number(ui.threads_per_group->value()));
    settings->setValue("preferences/messages_per_thread", QString::number(ui.messages_per_thread->value()));
    settings->setValue("preferences/show_more_count", QString::number(ui.show_more_count->value()));
    settings->setSignature(ui.signatureEdit->toPlainText());
    settings->sync();
    accept();
    deleteLater();
}
