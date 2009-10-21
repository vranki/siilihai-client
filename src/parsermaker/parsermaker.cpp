#include "parsermaker.h"

ParserMaker::ParserMaker(QWidget *parent) :
	QMainWindow(parent), pdb(this) {
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
	groupListEditor = new GroupListPatternEditor(session, parser, subscription, this);
	ui.tabWidget->addTab(groupListEditor, groupListEditor->tabIcon(), groupListEditor->tabName());
	threadListEditor = new ThreadListPatternEditor(session, parser, subscription, this);
	ui.tabWidget->addTab(threadListEditor, threadListEditor->tabIcon(), threadListEditor->tabName());
	threadListEditor->setEnabled(false);
	messageListEditor = new MessageListPatternEditor(session, parser, subscription, this);
	ui.tabWidget->addTab(messageListEditor, threadListEditor->tabIcon(), messageListEditor->tabName());
	messageListEditor->setEnabled(false);
	connect(groupListEditor, SIGNAL(groupSelected(ForumGroup)), threadListEditor, SLOT(setGroup(ForumGroup)));
	connect(threadListEditor, SIGNAL(threadSelected(ForumThread)), messageListEditor, SLOT(setThread(ForumThread)));

	connect(&protocol, SIGNAL(loginFinished(bool, QString)), this,
			SLOT(loginFinished(bool, QString)));
	connect(&protocol, SIGNAL(saveParserFinished(int, QString)), this,
			SLOT(saveParserFinished(int, QString)));

	protocol.login(settings.value("account/username", "").toString(),
			settings.value("account/password", "").toString());

	connect(ui.openParserButton, SIGNAL(clicked()), this, SLOT(openClicked()));
	connect(ui.newFromRequestButton, SIGNAL(clicked()), this, SLOT(newFromRequestClicked()));
	connect(ui.saveChangesButton, SIGNAL(clicked()), this, SLOT(saveClicked()));
	connect(ui.saveAsNewButton, SIGNAL(clicked()), this,
			SLOT(saveAsNewClicked()));
	connect(ui.testForumUrlButton, SIGNAL(clicked()), this,
			SLOT(testForumUrlClicked()));
	connect(ui.forumUrl, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
	connect(ui.parserName, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.viewThreadPath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.threadListPath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));
	connect(ui.viewMessagePath, SIGNAL(textEdited(QString)), this,
			SLOT(updateState()));

	subscription.latest_threads = 20;
	subscription.latest_messages = 20;

	updateState();
	ui.tabWidget->setCurrentIndex(0);
	show();
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

	parser.group_list_pattern = groupListEditor->pattern();
	parser.thread_list_pattern = threadListEditor->pattern();
	parser.message_list_pattern = messageListEditor->pattern();

	bool mayWork = parser.mayWork();

	ui.saveChangesButton->setEnabled(mayWork);
	ui.saveAsNewButton->setEnabled(mayWork);

	ui.baseUrlTL->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlVT->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlVM->setText(parser.forumUrlWithoutEnd());
	ui.baseUrlLI->setText(parser.forumUrlWithoutEnd());

	subscription.name = parser.parser_name;
	subscription.parser = parser.id;

	session.initialize(parser, subscription);
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

void ParserMaker::newFromRequestClicked() {
	OpenRequestDialog *dlg = new OpenRequestDialog(this, protocol);
	connect(dlg, SIGNAL(requestSelected(ForumRequest)), this,
			SLOT(requestSelected(ForumRequest)));
	dlg->show();
}

void ParserMaker::requestSelected(ForumRequest req) {
	ForumParser fp;
	fp.forum_url = req.forum_url;
	parserLoaded(fp);
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
	ui.charset->setEditText(p.charset);
	groupListEditor->setPattern(p.group_list_pattern);
	groupListEditor->parserUpdated();
	threadListEditor->setPattern(p.thread_list_pattern);
	threadListEditor->parserUpdated();
	messageListEditor->setPattern(p.message_list_pattern);
	messageListEditor->parserUpdated();
	ui.tabWidget->setCurrentIndex(1);
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

void ParserMaker::testForumUrlClicked() {
	QDesktopServices::openUrl(parser.forum_url);
}
