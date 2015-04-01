#include "patterneditor.h"

PatternEditor::PatternEditor(ParserEngine &eng, ForumParser *par, ForumSubscription *fos, QWidget *parent) :
    QWidget(parent), engine(eng), subscription(fos), parser(par), downloadParser(0), editTimeout(this) {
    ui.setupUi(this);
    matcher = new PatternMatcher(this, true);
    connect(ui.downloadButton, SIGNAL(clicked()), this, SLOT(downloadList()));
    connect(ui.testPageSpanning, SIGNAL(clicked()), this, SLOT(testPageSpanning()));
    connect(ui.viewInBrowserButton, SIGNAL(clicked()), this, SLOT(viewInBrowser()));
    connect(&editTimeout, SIGNAL(timeout()), this, SLOT(patternChanged()));
    connect(ui.patternEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited()));
    connect(matcher, SIGNAL(dataMatched(int, QString, PatternMatchType)), this, SLOT(dataMatched(int, QString, PatternMatchType)));
    connect(matcher, SIGNAL(dataMatchingStart(QString&)), this, SLOT(dataMatchingStart(QString&)));
    connect(matcher, SIGNAL(dataMatchingEnd()), this, SLOT(dataMatchingEnd()));
    connect(ui.resultsTable, SIGNAL(cellClicked(int,int)), this, SLOT(resultCellActivated(int, int)));
    connect(ui.resultsTable, SIGNAL(itemChanged ( QTableWidgetItem *)), this, SLOT(updateCount()));

    ui.resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.sourceTextEdit->setFontFamily("monospace");
    patternEditorCursor = ui.sourceTextEdit->textCursor();
    pageSpanningTest = false;
    editTimeout.setSingleShot(true);
    subscription = downloadSubscription = fos;
}

PatternEditor::~PatternEditor() {
}

QString PatternEditor::pattern() {
    return ui.patternEdit->text();
}

void PatternEditor::textEdited() {
    editTimeout.start(2000);
}

void PatternEditor::updateCount() {
    ui.countLabel->setText(QString("Found %1 matches").arg(ui.resultsTable->rowCount()));
}

void PatternEditor::setPattern(QString txt) {
    ui.patternEdit->setText(txt);
}

void PatternEditor::dataMatched(int pos, QString data, PatternMatchType type) {
    if(pageSpanningTest) return;

    QString myData = ui.sourceTextEdit->toPlainText().mid(pos, data.length());
    static int mismatches = 0;
    if(myData != data && mismatches < 5) {
        for(int i=0;i<data.length();i++) {
            if(myData[i] != data[i]) {
                int preDisplay = i-5;
                int postDisplay = i + 30;
                if(preDisplay < 0) preDisplay = 0;
                if(postDisplay > data.length()) postDisplay = data.length();
                qDebug() << "Data MisMatch at pos " << i << " pre " << preDisplay << ": \n" <<
                            myData.mid(preDisplay, postDisplay-preDisplay) <<
                            "\n != \n" << data.mid(preDisplay, postDisplay-preDisplay);
                qDebug() << "Problem char = " << myData.at(i) << " (" << (int) myData.at(i).toLatin1() <<  ") != "
                         << data.at(i) << "(" << (int) data.at(i).toLatin1() << ")";
                i = data.length();
                mismatches++;
            }
        }

        qDebug() << "Position: " << pos << " match type = " << type;
    }

    if(pageSpanningTest) return;

    QColor color;

    switch (type) {
    case PMTMatch:
        color = Qt::blue;
        break;
    case PMTNoMatch:
        color = Qt::black;
        break;
    case PMTTag:
        color = Qt::green;
        break;
    case PMTIgnored:
        color = Qt::darkGray;
        break;
    default:
        Q_ASSERT(false);
    }

    patternEditorCursor.setPosition(patternEditorCursor.position() + data.length(), QTextCursor::KeepAnchor);
    QTextCharFormat fmt = patternEditorCursor.charFormat();
    fmt.setForeground(QBrush(color));
    patternEditorCursor.setCharFormat(fmt);
    patternEditorCursor.setPosition(patternEditorCursor.position(), QTextCursor::MoveAnchor);
}


void PatternEditor::dataMatchingStart(QString &html) {
    if(pageSpanningTest) return;
    if (ui.sourceTextEdit->toPlainText().length() == 0) {
        ui.sourceTextEdit->setPlainText(html);
    }
    patternEditorCursor.setPosition(0, QTextCursor::MoveAnchor);
    pageSpanningTest = false;
}

void PatternEditor::dataMatchingEnd() {
    if(pageSpanningTest) return;
}


QIcon PatternEditor::tabIcon() {
    return QIcon();
}

QString PatternEditor::tabName() {
    return "?";
}

void PatternEditor::parserUpdated() {
}

void PatternEditor::viewInBrowser() {
    QDesktopServices::openUrl(QUrl(ui.urlLabel->text()));
}

void PatternEditor::listGroupsFinished(QList<ForumGroup*> groups) {
    Q_UNUSED(groups);
}

void PatternEditor::listMessagesFinished(QList<ForumMessage*> messages, ForumThread *thread) {
    Q_UNUSED(messages);
    Q_UNUSED(thread);
}
void PatternEditor::listThreadsFinished(QList<ForumThread*> threads, ForumGroup *group) {
    Q_UNUSED(threads);
    Q_UNUSED(group);
}

