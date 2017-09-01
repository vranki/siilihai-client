#include "grouplistpatterneditor.h"
#include <siilihai/forumdata/forumsubscription.h>

GroupListPatternEditor::GroupListPatternEditor(ParserEngine &eng,
                                               ForumParser *par,
                                               ForumSubscription *fos,
                                               QWidget *parent) :
PatternEditor(eng, par, fos, parent) {

    connect(&engine, SIGNAL(listGroupsFinished(QList<ForumGroup*> &, ForumSubscription *)), this,
            SLOT(listGroupsFinished(QList<ForumGroup*> &, ForumSubscription *)));
    ui.testPageSpanning->setEnabled(false);
    ui.patternLabel->setText("<b>%a</b>=id <b>%b</b>=name %c=last change");
    Q_ASSERT(fos);
    subscription = fos;
    engine.setSubscription(fos);
    engine.setParser(par);
    engine.setPatternMatcher(matcher);
}

GroupListPatternEditor::~GroupListPatternEditor() {
}

void GroupListPatternEditor::downloadList() {
    engine.cancelOperation();
    engine.setSubscription(subscription);
    engine.setParser(parser);
    engine.setPatternMatcher(matcher);
    ui.downloadButton->setEnabled(false);
    engine.doUpdateForum();
    ui.sourceTextEdit->clear();
}

void GroupListPatternEditor::testPageSpanning() { }

void GroupListPatternEditor::listGroupsFinished(QList<ForumGroup*> &groups, ForumSubscription *sub) {
    if(sub != subscription) return;
    groupSelected(nullptr);
    qDeleteAll(listGroups);
    listGroups.clear();
    ui.resultsTable->clear();
    ui.resultsTable->setRowCount(groups.size());
    ui.resultsTable->setColumnCount(3);
    QStringList headers;
    headers << "Id" << "Name" << "Last change";
    ui.resultsTable->setHorizontalHeaderLabels(headers);
    ui.downloadButton->setEnabled(true);

    int row = 0;
    for (ForumGroup *group : groups) {
        QTableWidgetItem *newItem = new QTableWidgetItem(group->id());
        ui.resultsTable->setItem(row, 0, newItem);
        listGroups[row] = new ForumGroup(subscription);
        listGroups[row]->copyFrom(group);
        newItem = new QTableWidgetItem(group->name());
        ui.resultsTable->setItem(row, 1, newItem);
        newItem = new QTableWidgetItem(group->lastchange());
        ui.resultsTable->setItem(row, 2, newItem);
        row++;
    }
    ui.resultsTable->resizeColumnsToContents();
}

void GroupListPatternEditor::resultCellActivated(int row, int column) {
    Q_UNUSED(column);

    ForumGroup *selectedGroup = 0;

    if (listGroups.contains(row)) {
        qDebug() << "Selected group " << listGroups.value(row)->toString();
        selectedGroup = listGroups.value(row);
    } else {
        qDebug() << "Unknown id @ row " << row;
    }
    emit groupSelected(selectedGroup);
}

void GroupListPatternEditor::parserUpdated() {
    ui.urlLabel->setText(parser->forum_url);

    QString errors, warnings;
    if(!parser->group_list_pattern.contains("%a") && !parser->group_list_pattern.contains("%A"))
        errors += "Group id (%a) missing\n";
    if(!parser->group_list_pattern.contains("%b"))
        errors += "Group name (%b) missing\n";
    if(!parser->group_list_pattern.contains("%c"))
        warnings += "Last change (%c) is recommended\n";

    if(errors.length()==0) {
        errors = "Pattern is ok.\nWhen finished, choose a group and proceed to\nThread List tab.";
    }
    ui.errorLabel->setText(errors);
    ui.warningLabel->setText(warnings);
}

void GroupListPatternEditor::patternChanged() {
    parser->group_list_pattern = pattern();
    engine.setParser(parser);
    QString glhtml = ui.sourceTextEdit->toPlainText();
    engine.performListGroups(glhtml);
    parserUpdated();
}

void GroupListPatternEditor::reset()
{
    QList<ForumGroup*> empty;
    listGroupsFinished(empty, subscription);
    emit groupSelected(nullptr);
}

QIcon GroupListPatternEditor::tabIcon() {
    return QIcon(":/data/folder.png");
}

QString GroupListPatternEditor::tabName() {
    return "Group List";
}
