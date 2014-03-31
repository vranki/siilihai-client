#ifndef USERACCOUNTDIALOG_H
#define USERACCOUNTDIALOG_H

#include <QDialog>
#include <siilihai/siilihaisettings.h>

namespace Ui {
    class UserAccountDialog;
}

class UserAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserAccountDialog(QWidget *parent, SiilihaiSettings *s);
    ~UserAccountDialog();
signals:
    void unregisterSiilihai();
private slots:
    virtual void accept();
    void unregisterClicked();
private:
    Ui::UserAccountDialog *ui;
    SiilihaiSettings *settings;
    QString origUsername, origPassword;
};

#endif // USERACCOUNTDIALOG_H
