#include "openrequestdialog.h"

#include <siilihai/forumrequest.h>

OpenRequestDialog::OpenRequestDialog(QWidget *parent, SiilihaiProtocol &p) :
    QDialog(parent), protocol(p) {
    ui.setupUi(this);

    connect(&protocol, SIGNAL(listRequestsFinished(QList <ForumRequest*>)),
            this, SLOT(listRequestsFinished(QList <ForumRequest*>)));
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));

    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    protocol.listRequests();
}

OpenRequestDialog::~OpenRequestDialog() {
    for(ForumRequest *request : tableItems.values())
        request->deleteLater();
    tableItems.clear();
}

void OpenRequestDialog::listRequestsFinished(QList<ForumRequest*> requests) {
    for(ForumRequest *request : tableItems.values())
        request->deleteLater();
    tableItems.clear();

    ui.tableWidget->clear();
    ui.tableWidget->setColumnCount(4);
    ui.tableWidget->setRowCount(requests.size());
    QStringList headers;
    headers << "URL" << "Comment" << "User" << "Date";
    ui.tableWidget->setHorizontalHeaderLabels(headers);

    int row = 0;
    for(ForumRequest *request : requests) {
        QTableWidgetItem *item = new QTableWidgetItem(request->forum_url);
        ui.tableWidget->setItem(row, 0, item);
        item = new QTableWidgetItem(request->comment);
        ui.tableWidget->setItem(row, 1, item);
        item = new QTableWidgetItem(request->user);
        ui.tableWidget->setItem(row, 2, item);
        item = new QTableWidgetItem(request->date);
        ui.tableWidget->setItem(row, 3, item);
        tableItems[row] = request;
        row++;
    }
    ui.okButton->setEnabled(true);
    ui.tableWidget->resizeColumnsToContents();
}

void OpenRequestDialog::okClicked() {
    if(ui.tableWidget->currentRow()<0) return;
    emit(requestSelected(tableItems[ui.tableWidget->currentRow()]));
    deleteLater();
}

void OpenRequestDialog::cancelClicked() {
    deleteLater();
}
