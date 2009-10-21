/*
 * grouplistpatterneditor.cpp
 *
 *  Created on: Oct 13, 2009
 *      Author: vranki
 */

#include "grouplistpatterneditor.h"

GroupListPatternEditor::GroupListPatternEditor(ForumSession &ses,
		ForumParser &par, ForumSubscription &fos, QWidget *parent) :
	PatternEditor(ses, par, fos, parent) {

	connect(&session, SIGNAL(listGroupsFinished(QList<ForumGroup>)), this,
			SLOT(listGroupsFinished(QList<ForumGroup>)));
	ui.testPageSpanning->setEnabled(false);
	ui.patternLabel->setText("<b>%a</b>=id <b>%b</b>=name %c=last change");
}

GroupListPatternEditor::~GroupListPatternEditor() {
}

void GroupListPatternEditor::downloadList() {
	qDebug() << Q_FUNC_INFO << " " << parser.group_list_pattern;

	session.initialize(parser, subscription, &matcher);

	ui.downloadButton->setEnabled(false);
	session.listGroups();
	ui.sourceTextEdit->clear();
}

void GroupListPatternEditor::testPageSpanning() {
}

void GroupListPatternEditor::listGroupsFinished(QList<ForumGroup> groups) {
	/*
	ForumGroup selectedGroup;
	selectedGroup.id = QString::null;
	emit(groupSelected(selectedGroup));
	*/
	listIds.clear();
	ui.resultsTable->clear();
	ui.resultsTable->setRowCount(groups.size());
	ui.resultsTable->setColumnCount(3);
	QStringList headers;
	headers << "Id" << "Name" << "Last change";
	ui.resultsTable->setHorizontalHeaderLabels(headers);
	ui.downloadButton->setEnabled(true);

	for (int i = 0; i < groups.size(); i++) {
		QTableWidgetItem *newItem = new QTableWidgetItem(groups[i].id);
		ui.resultsTable->setItem(i, 0, newItem);
		listIds[i] = groups[i].id;

		newItem = new QTableWidgetItem(groups[i].name);
		ui.resultsTable->setItem(i, 1, newItem);
		newItem = new QTableWidgetItem(groups[i].lastchange);
		ui.resultsTable->setItem(i, 2, newItem);
	}
	ui.resultsTable->resizeColumnsToContents();
}

void GroupListPatternEditor::resultCellActivated(int row, int column) {
	ForumGroup selectedGroup;
	selectedGroup.id = QString::null;

	if (listIds.contains(row)) {
		qDebug() << "Selected group " << listIds[row];
		selectedGroup.id = listIds[row];
		selectedGroup.parser = parser.id;

	} else {
		qDebug() << "Unknown id @ row " << row;
	}
	emit(groupSelected(selectedGroup));
}

void GroupListPatternEditor::parserUpdated() {
	ui.urlLabel->setText(parser.forum_url);

	QString errors, warnings;
	if(!parser.group_list_pattern.contains("%a") && !parser.group_list_pattern.contains("%A"))
		errors += "Group id (%a) missing\n";
	if(!parser.group_list_pattern.contains("%b"))
		errors += "Group name (%b) missing\n";
	if(!parser.group_list_pattern.contains("%c"))
		warnings += "Last change (%c) is recommended\n";

	if(errors.length()==0) {
		errors = "Pattern is ok.\nWhen finished, choose a group and proceed to\nThread List tab.";
	}
	ui.errorLabel->setText(errors);
	ui.warningLabel->setText(warnings);
}

void GroupListPatternEditor::patternChanged(QString txt) {
	parser.group_list_pattern = txt;
	session.setParser(parser);
	QString glhtml = ui.sourceTextEdit->toPlainText();
	session.performListGroups(glhtml);
	parserUpdated();
}

QIcon GroupListPatternEditor::tabIcon() {
	return QIcon(":/data/folder.png");
}

QString GroupListPatternEditor::tabName() {
	return "Group List";
}
