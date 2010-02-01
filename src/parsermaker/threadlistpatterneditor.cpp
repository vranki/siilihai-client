/*
 * threadlistpatterneditor.cpp
 *
 *  Created on: Oct 13, 2009
 *      Author: vranki
 */

#include "threadlistpatterneditor.h"

ThreadListPatternEditor::ThreadListPatternEditor(ForumSession &ses,
		ForumParser &par, ForumSubscription *fos, QWidget *parent) :
	PatternEditor(ses, par, fos, parent) {
	setEnabled(false);
	connect(&session,
			SIGNAL(listThreadsFinished(QList<ForumThread>, ForumGroup)), this,
			SLOT(listThreadsFinished(QList<ForumThread>, ForumGroup)));
	ui.patternLabel->setText("<b>%a</b>=id <b>%b</b>=name %c=last change");
	session.initialize(par, fos, matcher);
}

ThreadListPatternEditor::~ThreadListPatternEditor() {
}

QString ThreadListPatternEditor::tabName() {
	return "Thread List";
}

void ThreadListPatternEditor::downloadList() {
	downloadParser = parser;
	downloadParser.thread_list_page_increment = 0;
	downloadParser.view_thread_page_increment = 0;
	downloadSubscription = subscription;

	session.initialize(downloadParser, downloadSubscription, matcher);
	session.listThreads(currentGroup);

	ui.sourceTextEdit->clear();
	ui.downloadButton->setEnabled(false);
	ui.testPageSpanning->setEnabled(false);
	pageSpanningTest = false;
}

void ThreadListPatternEditor::testPageSpanning() {
	downloadParser = parser;
	downloadSubscription = subscription;
	downloadSubscription->setLatestThreads(999);
	downloadSubscription->setLatestMessages(999);

	session.initialize(downloadParser, downloadSubscription, matcher);
	session.listThreads(currentGroup);

	ui.sourceTextEdit->clear();
	ui.sourceTextEdit->append("Source not available when testing multiple pages.");
	ui.downloadButton->setEnabled(false);
	ui.testPageSpanning->setEnabled(false);
	pageSpanningTest = true;
}

void ThreadListPatternEditor::setGroup(ForumGroup *grp) {
	currentGroup = grp;
	setEnabled(currentGroup->id().length() > 0);
	ui.downloadButton->setEnabled(true);
	ui.testPageSpanning->setEnabled(true);
	parserUpdated();
}

void ThreadListPatternEditor::resultCellActivated(int row, int column) {
	ForumThread *selectedThread = 0;
	/* @todo fix
	selectedThread.id = QString::null;

	if (listIds.contains(row)) {
		qDebug() << "Selected thread " << listIds[row];
		selectedThread.id = listIds[row];
		selectedThread.groupid = currentGroup.id;
		selectedThread.forumid = parser.id;
	} else {
		qDebug() << "Unknown id @ row " << row;
	}
	*/
	emit(threadSelected(selectedThread));
}

void ThreadListPatternEditor::listThreadsFinished(QList<ForumThread*> threads,
		ForumGroup *group) {
	ForumThread selectedThread;
	listIds.clear();
	ui.resultsTable->clear();
	ui.resultsTable->setRowCount(threads.size());
	ui.resultsTable->setColumnCount(3);
	QStringList headers;
	headers << "Id" << "Name" << "Last change";
	ui.resultsTable->setHorizontalHeaderLabels(headers);
	ui.downloadButton->setEnabled(true);
	ui.testPageSpanning->setEnabled(true);

	for (int i = 0; i < threads.size(); i++) {
		QTableWidgetItem *newItem = new QTableWidgetItem(threads[i]->id());
		ui.resultsTable->setItem(i, 0, newItem);
		listIds[i] = threads[i]->id();

		newItem = new QTableWidgetItem(threads[i]->name());
		ui.resultsTable->setItem(i, 1, newItem);
		newItem = new QTableWidgetItem(threads[i]->lastchange());
		ui.resultsTable->setItem(i, 2, newItem);
	}

	ui.resultsTable->resizeColumnsToContents();
	ui.downloadButton->setEnabled(true);
	ui.testPageSpanning->setEnabled(true);
}

void ThreadListPatternEditor::parserUpdated() {
	if (currentGroup->id().length() > 0) {
		ui.urlLabel->setText(session.getThreadListUrl(currentGroup));
	} else {
		ui.urlLabel->setText("(No group selected)");
	}

	QString errors, warnings;
	if(!parser.thread_list_pattern.contains("%a") && !parser.thread_list_pattern.contains("%A"))
		errors += "Thread id (%a) missing\n";
	if(!parser.thread_list_pattern.contains("%b"))
		errors += "Thread name (%b) missing\n";
	if(!parser.thread_list_pattern.contains("%c"))
		warnings += "Last change (%c) is recommended\n";

	if(errors.length()==0) {
		errors = "Pattern is ok.\nRemember to test also\nmulti page spanning.\n"
			"When finished, choose a thread and proceed to\nGroup List tab.";
	}
	ui.errorLabel->setText(errors);
	ui.warningLabel->setText(warnings);
}

void ThreadListPatternEditor::patternChanged() {
	parser.thread_list_pattern = pattern();
	session.setParser(parser);
	if (!pageSpanningTest) {
		QString glhtml = ui.sourceTextEdit->toPlainText();
		session.performListThreads(glhtml);
	}
	parserUpdated();
}

QIcon ThreadListPatternEditor::tabIcon() {
	return QIcon(":/data/mail-unread.png");
}
