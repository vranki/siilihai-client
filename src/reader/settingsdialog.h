#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QSettings>

#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, QSettings *s);
    ~SettingsDialog();
public slots:
	void closeClicked();

private:
    Ui::SettingsDialogClass ui;
    QSettings *settings;
};

#endif // SETTINGSDIALOG_H
