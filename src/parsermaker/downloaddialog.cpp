#include "downloaddialog.h"

DownloadDialog::DownloadDialog(QWidget *parent, SiilihaiProtocol &p) :
	QDialog(parent), protocol(p) {
	ui.setupUi(this);
	connect(&protocol, SIGNAL(listParsersFinished(QList <ForumParser>)), this,
			SLOT(listParsersFinished(QList <ForumParser>)));
	connect(&protocol, SIGNAL(getParserFinished(ForumParser)), this,
			SLOT(getParserFinished(ForumParser)));
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	protocol.listParsers();
}

DownloadDialog::~DownloadDialog() {

}

void DownloadDialog::listParsersFinished(QList<ForumParser> parsers) {
	ui.listWidget->clear();
	allParsers = parsers;
	for (int i = 0; i < allParsers.size(); i++) {
		QListWidgetItem *item = new QListWidgetItem(ui.listWidget);
		item->setText(allParsers[i].parser_name);
		item->setToolTip(allParsers[i].forum_url);
		ui.listWidget->addItem(item);
		listWidgetItemForum[item] = &allParsers[i];
	}
	ui.okButton->setEnabled(true);
}

void DownloadDialog::getParserFinished(ForumParser parser) {
	if(parser.isSane()) {
		emit parserLoaded(parser);
		deleteLater();
	}
}

void DownloadDialog::okClicked() {
	if(ui.listWidget->selectedItems().size()==0) return;

	protocol.getParser(listWidgetItemForum[ui.listWidget->selectedItems()[0]]->id);
}

void DownloadDialog::cancelClicked() {
	deleteLater();
}
