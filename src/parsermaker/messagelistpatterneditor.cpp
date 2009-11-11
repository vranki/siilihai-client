/*
 * MessageListpatterneditor.cpp
 *
 *  Created on: Oct 13, 2009
 *      Author: vranki
 */

#include "messagelistpatterneditor.h"

MessageListPatternEditor::MessageListPatternEditor(ForumSession &ses,
		ForumParser &par, ForumSubscription &fos, QWidget *parent) :
	PatternEditor(ses, par, fos, parent) {
	setEnabled(false);
	connect(&session, SIGNAL(listMessagesFinished(QList<ForumMessage>,
					ForumThread)), this,
			SLOT(listMessagesFinished(QList<ForumMessage>,
							ForumThread)));
	ui.patternLabel->setText(
			"<b>%a</b>=id %b=subject <b>%c</b>=message body %d=author %e=last change");
}

MessageListPatternEditor::~MessageListPatternEditor() {
}

QString MessageListPatternEditor::tabName() {
	return "Message List";
}

void MessageListPatternEditor::downloadList() {
	downloadParser = parser;
	downloadParser.thread_list_page_increment = 0;
	downloadParser.view_thread_page_increment = 0;
	downloadSubscription = subscription;

	session.initialize(downloadParser, downloadSubscription, matcher);
	session.listMessages(currentThread);

	ui.sourceTextEdit->clear();
	ui.downloadButton->setEnabled(false);
	ui.testPageSpanning->setEnabled(false);
	pageSpanningTest = false;
}

void MessageListPatternEditor::testPageSpanning() {
	downloadParser = parser;
	downloadSubscription = subscription;
	downloadSubscription.latest_threads = 999;
	downloadSubscription.latest_messages = 999;

	session.initialize(downloadParser, downloadSubscription, matcher);
	session.listMessages(currentThread);

	ui.sourceTextEdit->clear();
	ui.sourceTextEdit->append(
			"Source not available when testing multiple pages.");
	ui.downloadButton->setEnabled(false);
	ui.testPageSpanning->setEnabled(false);
	pageSpanningTest = true;
}

void MessageListPatternEditor::setThread(ForumThread thread) {
	currentThread = thread;
	if (currentThread.id.length() > 0) {
		setEnabled(true);
		ui.downloadButton->setEnabled(true);
		ui.testPageSpanning->setEnabled(true);

		parserUpdated();
	}
}

void MessageListPatternEditor::resultCellActivated(int row, int column) {
	ForumThread selectedThread;
	selectedThread.id = QString::null;

	if (listIds.contains(row)) {
		QString id = listIds[row];
		QString body = bodies[listIds[row]];
		QMessageBox msgBox(this);
		msgBox.setText(body);
		msgBox.setModal(true);
		msgBox.exec();
	} else {
		qDebug() << "Unknown id @ row " << row;
	}
}

void MessageListPatternEditor::listMessagesFinished(
		QList<ForumMessage> messages, ForumThread thread) {
	listIds.clear();
	bodies.clear();
	ui.resultsTable->clear();
	ui.resultsTable->setRowCount(messages.size());
	ui.resultsTable->setColumnCount(5);

	QStringList headers;
	headers << "Id" << "Subject" << "Author" << "Last Change" << "Body";
	ui.resultsTable->setHorizontalHeaderLabels(headers);

	for (int i = 0; i < messages.size(); i++) {
		QTableWidgetItem *newItem = new QTableWidgetItem(messages[i].id);
		ui.resultsTable->setItem(i, 0, newItem);
		listIds[i] = messages[i].id;
		bodies[messages[i].id] = messages[i].body;

		newItem = new QTableWidgetItem(messages[i].subject);
		ui.resultsTable->setItem(i, 1, newItem);
		newItem = new QTableWidgetItem(messages[i].author);
		ui.resultsTable->setItem(i, 2, newItem);
		newItem = new QTableWidgetItem(messages[i].lastchange);
		ui.resultsTable->setItem(i, 3, newItem);
		newItem = new QTableWidgetItem(messages[i].body.left(15));
		ui.resultsTable->setItem(i, 4, newItem);
	}
	ui.resultsTable->resizeColumnsToContents();

	ui.downloadButton->setEnabled(true);
	ui.testPageSpanning->setEnabled(true);
}

void MessageListPatternEditor::parserUpdated() {
	if (currentThread.id.length() > 0) {
		QString mlu = session.getMessageListUrl(currentThread);
		ui.urlLabel->setText(mlu);
	} else {
		ui.urlLabel->setText("(No thread selected)");
	}

	QString errors, warnings;
	if (!parser.message_list_pattern.contains("%a")
			&& !parser.message_list_pattern.contains("%A"))
		errors += "Message id (%a) missing\n";
	if (!parser.message_list_pattern.contains("%b"))
		warnings += "Message subject (%b) is recommended\n";
	if (!parser.message_list_pattern.contains("%c"))
		errors += "Message body (%c) is missing\n";
	if (!parser.message_list_pattern.contains("%d"))
		warnings += "Message author (%d) is recommended\n";
	if (!parser.message_list_pattern.contains("%e"))
		warnings += "Last change (%e) is recommended\n";

	if (errors.length() == 0) {
		errors = "Pattern is ok.\nClick on message to display its body.\nRemember also to test\nmulti page spanning.";
	}
	ui.errorLabel->setText(errors);
	ui.warningLabel->setText(warnings);
}

void MessageListPatternEditor::patternChanged(QString txt) {
	parser.message_list_pattern = txt;
	downloadParser = parser;
	downloadParser.thread_list_page_increment = 0;
	downloadParser.view_thread_page_increment = 0;
	downloadSubscription = subscription;
	session.setParser(downloadParser);
	QString glhtml = ui.sourceTextEdit->toPlainText();
	session.performListMessages(glhtml);
	parserUpdated();
}

QIcon MessageListPatternEditor::tabIcon() {
	return QIcon(":/data/mail-unread.png");
}
