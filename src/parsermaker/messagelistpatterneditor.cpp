#include "messagelistpatterneditor.h"
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/forummessage.h>

MessageListPatternEditor::MessageListPatternEditor(ForumSession &ses,
                                                   ForumParser *par,
                                                   ForumSubscription *fos,
                                                   QWidget *parent) : PatternEditor(ses, par, fos, parent) {
    setEnabled(false);
    connect(&session, SIGNAL(listMessagesFinished(QList<ForumMessage*>&, ForumThread*, bool)),
            this, SLOT(listMessagesFinished(QList<ForumMessage*>&, ForumThread*, bool)));
    ui.patternLabel->setText("<b>%a</b>=id %b=subject <b>%c</b>=message body %d=author %e=last change");
    subscription = fos;
    Q_ASSERT(fos);
    session.initialize(par, fos, matcher);
    currentThread = 0;
}

MessageListPatternEditor::~MessageListPatternEditor() {
}

QString MessageListPatternEditor::tabName() {
    return "Message List";
}

void MessageListPatternEditor::downloadList() {
    downloadParser = (*parser);
    downloadParser.thread_list_page_increment = 0;
    downloadParser.view_thread_page_increment = 0;
    downloadSubscription = subscription;

    session.initialize(&downloadParser, downloadSubscription, matcher);
    session.listMessages(currentThread);

    ui.sourceTextEdit->clear();
    ui.downloadButton->setEnabled(false);
    ui.testPageSpanning->setEnabled(false);
    pageSpanningTest = false;
}

void MessageListPatternEditor::testPageSpanning() {
    downloadParser = (*parser);
    downloadSubscription = subscription;
    downloadSubscription->setLatestThreads(999);
    downloadSubscription->setLatestMessages(999);

    session.initialize(&downloadParser, downloadSubscription, matcher);
    session.listMessages(currentThread);

    ui.sourceTextEdit->clear();
    ui.sourceTextEdit->append("Source not available when testing multiple pages.");
    ui.downloadButton->setEnabled(false);
    ui.testPageSpanning->setEnabled(false);
    pageSpanningTest = true;
}

void MessageListPatternEditor::setThread(ForumThread *thread) {
    currentThread = thread;
    setEnabled(currentThread != 0);
    ui.downloadButton->setEnabled(currentThread != 0);
    ui.testPageSpanning->setEnabled(currentThread != 0);

    if (currentThread) {
        parserUpdated();
    }
}

void MessageListPatternEditor::resultCellActivated(int row, int column) {
    Q_UNUSED(column);
    if (bodies.contains(row)) {
        QString body = bodies[row];
        QMessageBox msgBox(this);
        msgBox.setText(body);
        msgBox.setModal(true);
        msgBox.exec();
    } else {
        qDebug() << "Unknown id @ row " << row;
    }
}

void MessageListPatternEditor::listMessagesFinished(QList<ForumMessage*> &messages, ForumThread *thread, bool more) {
    Q_UNUSED(thread);
    Q_UNUSED(more);
    bodies.clear();
    ui.resultsTable->clear();
    ui.resultsTable->setRowCount(messages.size());
    ui.resultsTable->setColumnCount(5);

    QStringList headers;
    headers << "Id" << "Subject" << "Author" << "Last Change" << "Body";
    ui.resultsTable->setHorizontalHeaderLabels(headers);

    int tableRow = 0;
    foreach(ForumMessage *fm, messages) {
        QTableWidgetItem *newItem = new QTableWidgetItem(fm->id());
        ui.resultsTable->setItem(tableRow, 0, newItem);
        //listMessages[tableRow] = fm;
        bodies[tableRow] = fm->body();

        newItem = new QTableWidgetItem(fm->name());
        ui.resultsTable->setItem(tableRow, 1, newItem);
        newItem = new QTableWidgetItem(fm->author());
        ui.resultsTable->setItem(tableRow, 2, newItem);
        newItem = new QTableWidgetItem(fm->lastchange());
        ui.resultsTable->setItem(tableRow, 3, newItem);
        newItem = new QTableWidgetItem(fm->body().left(15));
        ui.resultsTable->setItem(tableRow, 4, newItem);
        tableRow++;
    }
    ui.resultsTable->resizeColumnsToContents();

    ui.downloadButton->setEnabled(true);
    ui.testPageSpanning->setEnabled(true);
}

void MessageListPatternEditor::parserUpdated() {
    if (currentThread) {
        QString mlu = session.getMessageListUrl(currentThread);
        ui.urlLabel->setText(mlu);
    } else {
        ui.urlLabel->setText("(No thread selected)");
    }

    QString errors, warnings;
    if (!parser->message_list_pattern.contains("%a")
        && !parser->message_list_pattern.contains("%A"))
        errors += "Message id (%a) missing\n";
    if (!parser->message_list_pattern.contains("%b"))
        warnings += "Message subject (%b) is recommended\n";
    if (!parser->message_list_pattern.contains("%c"))
        errors += "Message body (%c) is missing\n";
    if (!parser->message_list_pattern.contains("%d"))
        warnings += "Message author (%d) is recommended\n";
    if (!parser->message_list_pattern.contains("%e"))
        warnings += "Last change (%e) is recommended\n";

    if (errors.length() == 0) {
        errors = "Pattern is ok.\nClick on message to display its body.\nRemember also to test\nmulti page spanning.";
    }
    ui.errorLabel->setText(errors);
    ui.warningLabel->setText(warnings);
}

void MessageListPatternEditor::patternChanged() {
    parser->message_list_pattern = pattern();
    downloadParser = *parser;
    downloadParser.thread_list_page_increment = 0;
    downloadParser.view_thread_page_increment = 0;
    downloadSubscription = subscription;
    session.setParser(&downloadParser);
    QString glhtml = ui.sourceTextEdit->toPlainText();
    session.performListMessages(glhtml);
    parserUpdated();
}

QIcon MessageListPatternEditor::tabIcon() {
    return QIcon(":/data/mail-unread.png");
}
