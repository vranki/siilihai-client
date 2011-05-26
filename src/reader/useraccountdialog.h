#ifndef USERACCOUNTDIALOG_H
#define USERACCOUNTDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
    class UserAccountDialog;
}

class UserAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserAccountDialog(QWidget *parent, QSettings *s);
    ~UserAccountDialog();
signals:
    void unregisterSiilihai();
private slots:
    virtual void done(int result);
    void unregisterClicked();
private:
    Ui::UserAccountDialog *ui;
    QSettings *settings;
    QString origUsername, origPassword;
};

#endif // USERACCOUNTDIALOG_H
