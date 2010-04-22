
#include "threadlistpatterneditor.h"

ThreadListPatternEditor::ThreadListPatternEditor(ForumSession &ses,
                                                 ForumParser &par,
                                                 ForumSubscription *fos,
                                                 QWidget *parent) :
PatternEditor(ses, par, fos, parent) {
    setEnabled(false);
    connect(&session,
            SIGNAL(listThreadsFinished(QList<ForumThread*> &, ForumGroup*)), this,
            SLOT(listThreadsFinished(QList<ForumThread*> &, ForumGroup*)));
    ui.patternLabel->setText("<b>%a</b>=id <b>%b</b>=name %c=last change");
    subscription = fos;
    Q_ASSERT(fos);
    session.initialize(par, fos, matcher);
    currentGroup = 0;
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
    setEnabled(currentGroup);
    ui.downloadButton->setEnabled(currentGroup);
    ui.testPageSpanning->setEnabled(currentGroup);
    parserUpdated();
}

void ThreadListPatternEditor::resultCellActivated(int row, int column) {
    Q_UNUSED(column);

    ForumThread *selectedThread = 0;

    if (listThreads.contains(row)) {
        qDebug() << "Selected thread " << listThreads.value(row)->toString();
        selectedThread = listThreads[row];
    } else {
        qDebug() << "Unknown id @ row " << row;
    }
    emit(threadSelected(selectedThread));
}

void ThreadListPatternEditor::listThreadsFinished(QList<ForumThread*>& threads, ForumGroup *group) {
    Q_UNUSED(group);

    qDeleteAll(listThreads);
    listThreads.clear();
    ui.resultsTable->clear();
    ui.resultsTable->setRowCount(threads.size());
    ui.resultsTable->setColumnCount(3);
    QStringList headers;
    headers << "Id" << "Name" << "Last change";
    ui.resultsTable->setHorizontalHeaderLabels(headers);
    ui.downloadButton->setEnabled(true);
    ui.testPageSpanning->setEnabled(true);

    int row = 0;
    foreach (ForumThread *thread, threads) {
        QTableWidgetItem *newItem = new QTableWidgetItem(thread->id());
        ui.resultsTable->setItem(row, 0, newItem);
        listThreads[row] = new ForumThread(*thread);

        newItem = new QTableWidgetItem(thread->name());
        ui.resultsTable->setItem(row, 1, newItem);
        newItem = new QTableWidgetItem(thread->lastchange());
        ui.resultsTable->setItem(row, 2, newItem);
        row++;
    }

    ui.resultsTable->resizeColumnsToContents();
    ui.downloadButton->setEnabled(true);
    ui.testPageSpanning->setEnabled(true);
}

void ThreadListPatternEditor::parserUpdated() {
    if (currentGroup) {
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
