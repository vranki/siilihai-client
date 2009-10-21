#include "patterneditor.h"

PatternEditor::PatternEditor(ForumSession &ses, ForumParser &par,
		ForumSubscription &fos, QWidget *parent) :
	QWidget(parent), session(ses), parser(par), subscription(fos), matcher(
			this, true) {
	ui.setupUi(this);
	connect(ui.downloadButton, SIGNAL(clicked()), this,
			SLOT(downloadList()));
	connect(ui.testPageSpanning, SIGNAL(clicked()), this,
			SLOT(testPageSpanning()));
	connect(ui.viewInBrowserButton, SIGNAL(clicked()), this,
			SLOT(viewInBrowser()));
	connect(ui.patternEdit, SIGNAL(textEdited(QString)), this,
			SLOT(patternChanged(QString)));
	connect(&matcher, SIGNAL(dataMatched(int, QString, PatternMatchType)),
			this, SLOT(dataMatched(int, QString, PatternMatchType)));
	connect(&matcher, SIGNAL(dataMatchingStart(QString&)), this,
			SLOT(dataMatchingStart(QString&)));
	connect(&matcher, SIGNAL(dataMatchingEnd()), this, SLOT(dataMatchingEnd()));
	connect(ui.resultsTable, SIGNAL(cellClicked(int,int)), this, SLOT(resultCellActivated(int, int)));

	ui.resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.sourceTextEdit->setFontFamily("monospace");
	groupListCursor = ui.sourceTextEdit->textCursor();
}

PatternEditor::~PatternEditor() {

}

QString PatternEditor::pattern() {
	return ui.patternEdit->text();
}

void PatternEditor::setPattern(QString txt) {
	ui.patternEdit->setText(txt);
}


void PatternEditor::dataMatched(int pos, QString data, PatternMatchType type) {
	//	Q_ASSERT(pos < ui.groupListSource->toPlainText().length() && pos > 0);
	// qDebug() << Q_FUNC_INFO << " " << type << " " << pos;
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

	groupListCursor.setPosition(groupListCursor.position() + data.length(),
			QTextCursor::KeepAnchor);
	QTextCharFormat fmt = groupListCursor.charFormat();
	;
	fmt.setForeground(QBrush(color));
	groupListCursor.setCharFormat(fmt);
	groupListCursor.setPosition(groupListCursor.position(),
			QTextCursor::MoveAnchor);
}


void PatternEditor::dataMatchingStart(QString &html) {
	if(pageSpanningTest) return;
	qDebug() << "DMStart size: " << html.length();
	if (ui.sourceTextEdit->toPlainText().length() == 0) {
		ui.sourceTextEdit->clear();
		ui.sourceTextEdit->insertPlainText(html);
	}
	groupListCursor.setPosition(0, QTextCursor::MoveAnchor);
	pageSpanningTest = false;
}

void PatternEditor::dataMatchingEnd() {
	if(pageSpanningTest) return;
	qDebug() << "DMEnd size: " << ui.sourceTextEdit->toPlainText().length();

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

void PatternEditor::listGroupsFinished(QList<ForumGroup> groups) {
}

void PatternEditor::listMessagesFinished(QList<ForumMessage> messages,
		ForumThread thread) {

}
void PatternEditor::listThreadsFinished(QList<ForumThread> threads,
		ForumGroup group) {
}
