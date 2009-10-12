#include "parsermaker.h"

ParserMaker::ParserMaker(QWidget *parent) :
	QMainWindow(parent), pdb(this), matcher(this, true) {
	ui.setupUi(this);

	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QDir::homePath() + DATABASE_FILE);
	if (!db.open()) {
		QMessageBox msgBox;
		msgBox.setText("Error: Unable to open database.");
		msgBox.setModal(true);
		msgBox.exec();
		QCoreApplication::quit();
		return;
	}
	QString baseUrl = settings.value("network/baseurl", BASEURL).toString();
	protocol.setBaseURL(baseUrl);
	pdb.openDatabase();
	if (settings.value("account/username", "").toString() == "") {
		QMessageBox msgBox;
		msgBox.setText(
				"You don't have a valid Siilihai account.\n Set up one using Siilihai client.");
		msgBox.exec();
		QCoreApplication::quit();
		return;
	}
	connect(&protocol, SIGNAL(loginFinished(bool, QString)), this,
			SLOT(loginFinished(bool, QString)));
	connect(&protocol, SIGNAL(saveParserFinished(int, QString)), this,
			SLOT(saveParserFinished(int, QString)));
	connect(&matcher, SIGNAL(dataMatched(int, QString, PatternMatchType)),
			this, SLOT(dataMatched(int, QString, PatternMatchType)));
	connect(&matcher, SIGNAL(dataMatchingStart(QString&)), this,
			SLOT(dataMatchingStart(QString&)));
	connect(&matcher, SIGNAL(dataMatchingEnd()), this, SLOT(dataMatchingEnd()));
	connect(&session, SIGNAL(listGroupsFinished(QList<ForumGroup>)), this,
			SLOT(listGroupsFinished(QList<ForumGroup>)));

	protocol.login(settings.value("account/username", "").toString(),
			settings.value("account/password", "").toString());

	connect(ui.openParserButton, SIGNAL(clicked()), this, SLOT(openClicked()));
	connect(ui.saveChangesButton, SIGNAL(clicked()), this, SLOT(saveClicked()));
	connect(ui.saveAsNewButton, SIGNAL(clicked()), this,
			SLOT(saveAsNewClicked()));
	connect(ui.downloadGroupList, SIGNAL(clicked()), this,
			SLOT(downloadGroupList()));
	connect(ui.forumUrl, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
	connect(ui.parserName, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.viewThreadPath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.threadListPath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.viewMessagePath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.groupListPattern, SIGNAL(textEdited(QString)), this,
			SLOT(groupListPatternChanged(QString)));

	updateState();
	ui.tabWidget->setCurrentIndex(0);
	ui.groupListSource->setFontFamily("monospace");
	show();

	groupListCursor = ui.groupListSource->textCursor();
}

ParserMaker::~ParserMaker() {

}

void ParserMaker::updateState() {
	parser.parser_name = ui.parserName->text();
	parser.charset = ui.charset->currentText().toLower();
	parser.forum_url = ui.forumUrl->text();
	parser.parser_type = ui.parserType->currentIndex();
	parser.thread_list_path = ui.threadListPath->text();
	parser.view_thread_path = ui.viewThreadPath->text();
	parser.view_message_path = ui.viewMessagePath->text();
	parser.view_thread_page_start = ui.viewThreadPageStart->text().toInt();
	parser.view_thread_page_increment
			= ui.viewThreadPageIncrement->text().toInt();
	parser.thread_list_page_start = ui.threadListPageStart->text().toInt();
	parser.thread_list_page_increment
			= ui.threadListPageIncrement->text().toInt();
	parser.group_list_pattern = ui.groupListPattern->text();

	bool mayWork = parser.mayWork();

	ui.saveChangesButton->setEnabled(mayWork);
	ui.saveAsNewButton->setEnabled(mayWork);

	ui.baseUrlTL->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlVT->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlVM->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlLI->setText(parser.forumUrlWithoutEnd());

	subscription.name = parser.parser_name;
	subscription.parser = parser.id;
	subscription.latest_threads = 20;
	subscription.latest_messages = 20;

	session.initialize(parser, subscription, &matcher);
}

void ParserMaker::loginFinished(bool success, QString motd) {
	if (success) {
		qDebug() << "Login success.";
		ui.statusbar->showMessage("Logged in to Siilihai", 5000);
	} else {
		QMessageBox msgBox;
		if (motd.length() > 0) {
			msgBox.setText(motd);
		} else {
			msgBox.setText(
					"Error: Login failed. Check your username, password and network connection.");
		}
		msgBox.exec();
		QCoreApplication::quit();
	}
	disconnect(&protocol, SIGNAL(loginFinished(bool, QString)));
}

void ParserMaker::openClicked() {
	DownloadDialog *dlg = new DownloadDialog(this, protocol);
	connect(dlg, SIGNAL(parserLoaded(ForumParser)), this,
			SLOT(parserLoaded(ForumParser)));
	dlg->show();
}

void ParserMaker::parserLoaded(ForumParser p) {
	parser = p;
	ui.parserName->setText(p.parser_name);
	ui.forumUrl->setText(p.forum_url);
	ui.parserType->setCurrentIndex(p.parser_type);
	ui.forumSoftware->setText(p.forum_software);
	ui.threadListPath->setText(p.thread_list_path);
	ui.viewThreadPath->setText(p.view_thread_path);
	ui.viewThreadPageStart->setText(QString().number(p.view_thread_page_start));
	ui.viewThreadPageIncrement->setText(QString().number(
			p.view_thread_page_increment));
	ui.threadListPageStart->setText(QString().number(p.thread_list_page_start));
	ui.threadListPageIncrement->setText(QString().number(
			p.thread_list_page_increment));
	ui.groupListPattern->setText(p.group_list_pattern);

	updateState();
	ui.statusbar->showMessage("Parser loaded", 5000);
}

void ParserMaker::saveClicked() {
	QMessageBox msgBox;
	msgBox.setText("Are you sure you want to save changes?");
	msgBox.setInformativeText(
			"Note: This will fail, if you don't have rights to make changes to this parser.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	if (msgBox.exec() == QMessageBox::Yes) {
		protocol.saveParser(parser);
	}
}

void ParserMaker::saveAsNewClicked() {
	QMessageBox msgBox;
	msgBox.setText("This will upload parser as a new parser to Siilihai.");
	msgBox.setDetailedText(
			"Your parser will initially be marked 'new'. New parsers "
				"are not visible in public lists until another user has verified them "
				"as working. You should ask another user or Siilihai staff to check "
				"your parser.");
	msgBox.setInformativeText("Are you sure you wish to do this?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
	if (msgBox.exec() == QMessageBox::Yes) {
		parser.id = -1;
		protocol.saveParser(parser);
	}
}

void ParserMaker::saveParserFinished(int id, QString msg) {
	qDebug() << "Savefinished " << id;
	QString message;
	if (msg.length() > 0)
		message = msg;

	if (id > 0) {
		parser.id = id;
		if (message.isNull())
			message = "Parser saved.";
	} else {
		if (message.isNull())
			message = "Unable to save parser.";
	}
	QMessageBox msgBox;
	msgBox.setText(message);
	msgBox.exec();
}

void ParserMaker::downloadGroupList() {
	updateState();
	ui.downloadGroupList->setEnabled(false);
	session.listGroups();
	ui.groupListSource->clear();
	groupListDownloaded = false;
}

void ParserMaker::dataMatched(int pos, QString data, PatternMatchType type) {
//	Q_ASSERT(pos < ui.groupListSource->toPlainText().length() && pos > 0);
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

void ParserMaker::listGroupsFinished(QList<ForumGroup> groups) {
	ui.groupListResultsTable->clear();
	ui.groupListResultsTable->setRowCount(groups.size());
	QStringList headers;
	headers << "Id" << "Name" << "Last change";
	ui.groupListResultsTable->setHorizontalHeaderLabels(headers);
	ui.downloadGroupList->setEnabled(true);

	for (int i = 0; i < groups.size(); i++) {
		QTableWidgetItem *newItem = new QTableWidgetItem(groups[i].id);
		ui.groupListResultsTable->setItem(i, 0, newItem);
		newItem = new QTableWidgetItem(groups[i].name);
		ui.groupListResultsTable->setItem(i, 1, newItem);
		newItem = new QTableWidgetItem(groups[i].lastchange);
		ui.groupListResultsTable->setItem(i, 2, newItem);
	}
	ui.groupListResultsTable->resizeColumnsToContents();
}

void ParserMaker::groupListPatternChanged(QString txt) {
	parser.group_list_pattern = txt;
	session.setParser(parser);
	QString glhtml = ui.groupListSource->toPlainText();
	session.performListGroups(glhtml);
}

void ParserMaker::listThreadsFinished(QList<ForumThread> threads,
		ForumGroup group) {

}

void ParserMaker::listMessagesFinished(QList<ForumMessage> messages,
		ForumThread thread) {

}

void ParserMaker::dataMatchingStart(QString &html) {
	qDebug() << "DMStart " << html.length();
	if(ui.groupListSource->toPlainText().length() == 0) {
		ui.groupListSource->clear();
		ui.groupListSource->insertPlainText(html);
	}
	groupListCursor.setPosition(0, QTextCursor::MoveAnchor);
}

void ParserMaker::dataMatchingEnd() {
	groupListDownloaded = true;
}
