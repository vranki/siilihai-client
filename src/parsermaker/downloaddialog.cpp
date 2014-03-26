#include "downloaddialog.h"
#include <QMessageBox>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/parser/forumparser.h>

DownloadDialog::DownloadDialog(QWidget *parent, SiilihaiProtocol &p) :
    QDialog(parent), protocol(p) {
    ui.setupUi(this);
    connect(&protocol, SIGNAL(listForumsFinished(QList <ForumSubscription*>)), this, SLOT(listForumsFinished(QList <ForumSubscription*>)));
    connect(&protocol, SIGNAL(getParserFinished(ForumParser*)), this, SLOT(getParserFinished(ForumParser*)));
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    protocol.listForums();
}

DownloadDialog::~DownloadDialog() {
    qDeleteAll(allForums);
}

// @todo probably broken
void DownloadDialog::listForumsFinished(QList<ForumSubscription*> forums) {
    ui.listWidget->clear();
    allForums = forums;
    foreach(ForumSubscription *forum, allForums) {
        QListWidgetItem *item = new QListWidgetItem(ui.listWidget);
        item->setText(forum->alias());
        item->setToolTip(forum->forumUrl().toString());
        ui.listWidget->addItem(item);
        listWidgetItemForum[item] = forum;
    }
    ui.okButton->setEnabled(true);
}

void DownloadDialog::getParserFinished(ForumParser *parser) {
    if(parser && parser->isSane()) {
        emit parserLoaded(parser);
        deleteLater();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Failed to download parser");
        msgBox.exec();
    }
}

void DownloadDialog::okClicked() {
    if(ui.listWidget->selectedItems().size()==0) return;

    protocol.getParser(listWidgetItemForum[ui.listWidget->selectedItems()[0]]->id());
}

void DownloadDialog::cancelClicked() {
    deleteLater();
}
