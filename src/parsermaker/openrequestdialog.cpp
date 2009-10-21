#include "openrequestdialog.h"

OpenRequestDialog::OpenRequestDialog(QWidget *parent, SiilihaiProtocol &p) :
	QDialog(parent), protocol(p) {
	ui.setupUi(this);

	connect(&protocol, SIGNAL(listRequestsFinished(QList <ForumRequest>)),
			this, SLOT(listRequestsFinished(QList <ForumRequest>)));
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));

	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	protocol.listRequests();
}

OpenRequestDialog::~OpenRequestDialog() {

}

void OpenRequestDialog::listRequestsFinished(QList<ForumRequest> requests) {
	ui.tableWidget->clear();
	ui.tableWidget->setColumnCount(4);
	ui.tableWidget->setRowCount(requests.size());
	QStringList headers;
	headers << "URL" << "Comment" << "User" << "Date";
	ui.tableWidget->setHorizontalHeaderLabels(headers);

	for (int i = 0; i < requests.size(); i++) {
		QTableWidgetItem *item = new QTableWidgetItem(requests[i].forum_url);
		ui.tableWidget->setItem(i, 0, item);
		item = new QTableWidgetItem(requests[i].comment);
		ui.tableWidget->setItem(i, 1, item);
		item = new QTableWidgetItem(requests[i].user);
		ui.tableWidget->setItem(i, 2, item);
		item = new QTableWidgetItem(requests[i].date);
		ui.tableWidget->setItem(i, 3, item);
		tableItems[i] = requests[i];
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
