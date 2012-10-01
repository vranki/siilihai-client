#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QtGui/QDialog>

#include <siilihai/siilihaiprotocol.h>
#include <siilihai/parser/forumparser.h>

#include "ui_downloaddialog.h"

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QWidget *parent, SiilihaiProtocol &p);
    ~DownloadDialog();

public slots:
    void listParsersFinished(QList <ForumParser*> parsers);
    void getParserFinished(ForumParser *parser);
    void okClicked();
    void cancelClicked();
signals:
    void parserLoaded(ForumParser *p); // Emitted when parser was d/l'd. Will be deleted.

private:
    Ui::DownloadDialogClass ui;
    SiilihaiProtocol &protocol;
    QHash <QListWidgetItem*, ForumParser*> listWidgetItemForum;
    QList <ForumParser*> allParsers;

};

#endif // DOWNLOADDIALOG_H
