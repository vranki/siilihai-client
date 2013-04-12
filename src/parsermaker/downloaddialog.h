#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>
#include <siilihai/siilihaiprotocol.h>
#include "ui_downloaddialog.h"

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QWidget *parent, SiilihaiProtocol &p);
    ~DownloadDialog();

public slots:
    void listForumsFinished(QList <ForumSubscription*> forums);
    void getParserFinished(ForumParser *parser);
    void okClicked();
    void cancelClicked();
signals:
    void parserLoaded(ForumParser *p); // Emitted when parser was d/l'd. Will be deleted.

private:
    Ui::DownloadDialogClass ui;
    SiilihaiProtocol &protocol;
    QHash <QListWidgetItem*, ForumSubscription*> listWidgetItemForum;
    QList <ForumSubscription*> allForums;

};

#endif // DOWNLOADDIALOG_H
