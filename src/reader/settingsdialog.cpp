#include "settingsdialog.h"
#include <siilihai/siilihaisettings.h>

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
    ui.threads_per_group->setMaximum(settings->maxThreadsPerGroup());
    ui.messages_per_thread->setValue(settings->messagesPerThread());
    ui.messages_per_thread->setMaximum(settings->maxMessagesPerThread());
    ui.show_more_count->setValue(settings->showMoreCount());
    ui.signatureEdit->setPlainText(settings->signature());
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::closeClicked() {
    settings->setSyncEnabled(ui.enableSyncing->isChecked());
    settings->setHttpProxy(ui.httpProxy->text());
    settings->setThreadsPerGroup(ui.threads_per_group->value());
    settings->setMessagesPerThread(ui.messages_per_thread->value());
    settings->setShowMoreCount(ui.show_more_count->value());
    settings->setSignature(ui.signatureEdit->toPlainText());
    settings->sync();
    accept();
    deleteLater();
}
