#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// Qt5 
#include <QDialog>
#include <siilihai/siilihaisettings.h>

#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, SiilihaiSettings *s);
    ~SettingsDialog();
public slots:
    void closeClicked();

private:
    Ui::SettingsDialogClass ui;
    SiilihaiSettings *settings;
};

#endif // SETTINGSDIALOG_H
