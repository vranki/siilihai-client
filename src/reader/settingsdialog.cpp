#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QSettings *s)
    : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
	settings = s;
	ui.updateAutomatically->setChecked(settings->value("preferences/update_automatically", false).toBool());
	ui.enableSyncing->setChecked(settings->value("preferences/sync_enabled", false).toBool());
	ui.httpProxy->setText(settings->value("preferences/http_proxy", "").toString());
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::closeClicked() {
	settings->setValue("preferences/update_automatically", ui.updateAutomatically->isChecked());
	settings->setValue("preferences/sync_enabled", ui.enableSyncing->isChecked());
	settings->setValue("preferences/http_proxy", ui.httpProxy->text());
	close();
}
