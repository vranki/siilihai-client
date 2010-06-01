#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QSettings *s)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
    settings = s;
    ui.updateAutomatically->setChecked(settings->value("preferences/update_automatically", true).toBool());
    ui.enableSyncing->setChecked(settings->value("preferences/sync_enabled", false).toBool());
    ui.httpProxy->setText(settings->value("preferences/http_proxy", "").toString());
    ui.threads_per_group->setValue(settings->value("preferences/threads_per_group", 20).toInt());
    ui.messages_per_thread->setValue(settings->value("preferences/messages_per_thread", 20).toInt());
    ui.show_more_count->setValue(settings->value("preferences/show_more_count", 30).toInt());
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::closeClicked() {
    settings->setValue("preferences/update_automatically", ui.updateAutomatically->isChecked());
    settings->setValue("preferences/sync_enabled", ui.enableSyncing->isChecked());
    settings->setValue("preferences/http_proxy", ui.httpProxy->text());
    settings->setValue("preferences/threads_per_group", QString::number(ui.threads_per_group->value()));
    settings->setValue("preferences/messages_per_thread", QString::number(ui.messages_per_thread->value()));
    settings->setValue("preferences/show_more_count", QString::number(ui.show_more_count->value()));
    settings->sync();
    accept();
    deleteLater();
}
