#include "parsermaker.h"
#include <siilihai/parser/parserdatabase.h>
#include <siilihai/parser/forumparser.h>
#include <siilihai/parser/forumsession.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/parser/parsermanager.h>
#include <siilihai/forumrequest.h>
#include <siilihai/credentialsrequest.h>
#include <QCloseEvent>

ParserMaker::ParserMaker(QWidget *parent, ParserManager *pd, QSettings &s, SiilihaiProtocol &p) :
    QMainWindow(parent), pdb(pd), settings(s), protocol(p), nam(this), session(this, &nam) {
    ui.setupUi(this);
    loginMatcher = new PatternMatcher(this, true);

    groupListEditor = new GroupListPatternEditor(session, &parser, &subscription, this);
    ui.tabWidget->addTab(groupListEditor, groupListEditor->tabIcon(), groupListEditor->tabName());
    threadListEditor = new ThreadListPatternEditor(session, &parser, &subscription, this);
    ui.tabWidget->addTab(threadListEditor, threadListEditor->tabIcon(), threadListEditor->tabName());
    threadListEditor->setEnabled(false);
    messageListEditor = new MessageListPatternEditor(session, &parser, &subscription, this);
    ui.tabWidget->addTab(messageListEditor, threadListEditor->tabIcon(), messageListEditor->tabName());
    messageListEditor->setEnabled(false);

    connect(groupListEditor, SIGNAL(groupSelected(ForumGroup*)), threadListEditor, SLOT(setGroup(ForumGroup*)));
    connect(threadListEditor, SIGNAL(threadSelected(ForumThread*)), messageListEditor, SLOT(setThread(ForumThread*)));

    connect(&protocol, SIGNAL(saveParserFinished(int, QString)), this, SLOT(saveParserFinished(int, QString)));
    connect(ui.openParserButton, SIGNAL(clicked()), this, SLOT(openClicked()));
    connect(ui.newFromRequestButton, SIGNAL(clicked()), this, SLOT(newFromRequestClicked()));
    connect(ui.saveChangesButton, SIGNAL(clicked()), this, SLOT(saveClicked()));
    connect(ui.saveAsNewButton, SIGNAL(clicked()), this, SLOT(saveAsNewClicked()));
    connect(ui.testForumUrlButton, SIGNAL(clicked()), this, SLOT(testForumUrlClicked()));
    connect(ui.forumUrl, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.parserName, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.parserType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()));
    connect(ui.viewThreadPath, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.threadListPath, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.viewMessagePath, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.loginPath, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.loginTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()));
    connect(ui.tryLoginButton, SIGNAL(clicked()), this, SLOT(tryLogin()));
    connect(ui.tryWithoutLoginButton, SIGNAL(clicked()), this, SLOT(tryWithoutLogin()));
    connect(ui.verifyLoginPattern, SIGNAL(textEdited(QString)), this, SLOT(updateState()));
    connect(ui.helpButton, SIGNAL(clicked()), this, SLOT(helpClicked()));
    connect(&session, SIGNAL(loginFinished(ForumSubscription *,bool)), this, SLOT(loginFinished(ForumSubscription *, bool)));
    connect(&session, SIGNAL(networkFailure(QString)), this, SLOT(networkFailure(QString)));
    connect(&session, SIGNAL(getHttpAuthentication(ForumSubscription *, QAuthenticator *)),
            this, SLOT(getHttpAuthentication(ForumSubscription *, QAuthenticator *)));

    connect(loginMatcher, SIGNAL(dataMatched(int, QString, PatternMatchType)),
            this, SLOT(dataMatched(int, QString, PatternMatchType)));
    connect(loginMatcher, SIGNAL(dataMatchingStart(QString&)), this, SLOT(dataMatchingStart(QString&)));
    connect(loginMatcher, SIGNAL(dataMatchingEnd()), this, SLOT(dataMatchingEnd()));

    subscription.setLatestThreads(100);
    subscription.setLatestMessages(100);
    loginWithoutCredentials = false;

    updateState();
    ui.tabWidget->setCurrentIndex(0);
    if (!restoreGeometry(settings.value("parsermaker_geometry").toByteArray()))
        showMaximized();

    show();
}

ParserMaker::~ParserMaker() {
}

void ParserMaker::updateState() {
    parser.setName(ui.parserName->text());
    parser.charset = ui.charset->currentText().toLower();
    parser.forum_url = ui.forumUrl->text();
    parser.forum_software = ui.forumSoftware->text();
    parser.parser_type = ui.parserType->currentIndex();
    parser.login_type = (ForumParser::ForumLoginType) ui.loginTypeCombo->currentIndex();
    parser.login_path = ui.loginPath->text();
    parser.verify_login_pattern = ui.verifyLoginPattern->text();
    parser.login_parameters = ui.loginParameters->text();
    parser.thread_list_path = ui.threadListPath->text();
    parser.view_thread_path = ui.viewThreadPath->text();
    parser.view_message_path = ui.viewMessagePath->text();
    parser.view_thread_page_start = ui.viewThreadPageStart->text().toInt();
    parser.view_thread_page_increment = ui.viewThreadPageIncrement->text().toInt();
    parser.thread_list_page_start = ui.threadListPageStart->text().toInt();
    parser.thread_list_page_increment = ui.threadListPageIncrement->text().toInt();

    parser.group_list_pattern = groupListEditor->pattern();
    parser.thread_list_pattern = threadListEditor->pattern();
    parser.message_list_pattern = messageListEditor->pattern();

    bool mayWork = parser.mayWork();

    ui.saveChangesButton->setEnabled(mayWork && parser.id() > 0  && !settings.value("account/noaccount", false).toBool());
    ui.saveAsNewButton->setEnabled(mayWork && !settings.value("account/noaccount", false).toBool());

    ui.loginUrlLabel->setText(ui.loginPath->text());
    ui.loginTypeCombo->setEnabled(ui.loginPath->text().length() > 0);
    ui.tryLoginButton->setEnabled(parser.supportsLogin());
    ui.tryWithoutLoginButton->setEnabled(parser.supportsLogin());

    ui.baseUrlTL->setText(parser.forumUrlWithoutEnd());
    ui.baseUrlVT->setText(parser.forumUrlWithoutEnd());
    ui.baseUrlVM->setText(parser.forumUrlWithoutEnd());
    ui.baseUrlLI->setText(parser.forumUrlWithoutEnd());

    subscription.setAlias(parser.name());
    subscription.setParser(parser.id());
    if (loginWithoutCredentials) {
        subscription.setUsername(QString::null);
        subscription.setPassword(QString::null);
    } else {
        subscription.setUsername(ui.usernameEdit->text());
        subscription.setPassword(ui.passwordEdit->text());
    }
    session.setParser(&parser);
    groupListEditor->parserUpdated();
    threadListEditor->parserUpdated();
    messageListEditor->parserUpdated();

    ui.tryLoginButton->setEnabled(true);
    ui.tryWithoutLoginButton->setEnabled(true);
}

void ParserMaker::openClicked() {
    DownloadDialog *dlg = new DownloadDialog(this, protocol);
    connect(dlg, SIGNAL(parserLoaded(ForumParser*)), this, SLOT(parserLoaded(ForumParser*)));
    dlg->show();
}

void ParserMaker::newFromRequestClicked() {
    OpenRequestDialog *dlg = new OpenRequestDialog(this, protocol);
    connect(dlg, SIGNAL(requestSelected(ForumRequest*)), this, SLOT(requestSelected(ForumRequest*)));
    dlg->show();
}

void ParserMaker::requestSelected(ForumRequest *req) {
    ForumParser *fp = new ForumParser(this);
    fp->forum_url = req->forum_url;
    parserLoaded(fp);
}

void ParserMaker::parserLoaded(ForumParser *p) {
    parser = *p;
    ui.parserName->setText(p->name());
    ui.forumUrl->setText(p->forum_url);
    ui.parserType->setCurrentIndex(p->parser_type);
    ui.forumSoftware->setText(p->forum_software);
    ui.threadListPath->setText(p->thread_list_path);
    ui.viewThreadPath->setText(p->view_thread_path);
    ui.viewThreadPageStart->setText(QString().number(p->view_thread_page_start));
    ui.viewThreadPageIncrement->setText(QString().number(p->view_thread_page_increment));
    ui.threadListPageStart->setText(QString().number(p->thread_list_page_start));
    ui.threadListPageIncrement->setText(QString().number(p->thread_list_page_increment));
    ui.viewMessagePath->setText(p->view_message_path);
    ui.charset->setEditText(p->charset);
    ui.loginPath->setText(p->login_path);
    ui.loginTypeCombo->setCurrentIndex(p->login_type);
    ui.loginParameters->setText(p->login_parameters);
    ui.verifyLoginPattern->setText(p->verify_login_pattern);
    groupListEditor->setPattern(p->group_list_pattern);
    groupListEditor->parserUpdated();
    threadListEditor->setPattern(p->thread_list_pattern);
    threadListEditor->parserUpdated();
    messageListEditor->setPattern(p->message_list_pattern);
    messageListEditor->parserUpdated();
    ui.tabWidget->setCurrentIndex(1);
    updateState();
    ui.statusbar->showMessage("Parser loaded", 5000);
}

void ParserMaker::saveClicked() {
    updateState();
    QMessageBox msgBox(this);
    msgBox.setText("Are you sure you want to save changes?");
    msgBox.setInformativeText("Note: This will fail, if you don't have rights to make changes to this parser.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if (msgBox.exec() == QMessageBox::Yes) {
        protocol.saveParser(&parser);
    }
}

void ParserMaker::saveAsNewClicked() {
    updateState();
    QMessageBox msgBox(this);
    msgBox.setText("This will upload parser as a new parser to Siilihai.");
    msgBox.setDetailedText(
                "Your parser will initially be marked 'new'. New parsers "
                "may not be visible in public lists until another user has verified them "
                "as working. You should ask another user or Siilihai staff to check "
                "your parser.");
    msgBox.setInformativeText("Are you sure you wish to do this?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if (msgBox.exec() == QMessageBox::Yes) {
        parser.setId(-1);
        protocol.saveParser(&parser);
    }
}

void ParserMaker::saveParserFinished(int id, QString msg) {
    QString message;
    if (msg.length() > 0)
        message = msg;

    if (id > 0) {
        parser.setId(id);
        if (message.isNull())
            message = "Parser saved";
        emit(parserSaved(&parser));
    } else {
        if (message.isNull())
            message = "Unable to save parser";
    }
    QMessageBox msgBox(this);
    msgBox.setText(message);
    msgBox.exec();
    updateState();
}

void ParserMaker::testForumUrlClicked() {
    QDesktopServices::openUrl(parser.forum_url);
}

void ParserMaker::closeEvent(QCloseEvent *event) {
    settings.setValue("parsermaker_geometry", saveGeometry());
    event->accept();
    deleteLater();
}

void ParserMaker::tryLogin() {
    session.initialize(&parser, &subscription, 0);
    loginWithoutCredentials = false;
    updateState();
    connect(&session, SIGNAL(receivedHtml(const QString&)), ui.loginTextEdit, SLOT(setPlainText(const QString&)));
    session.loginToForum();
    ui.tryLoginButton->setEnabled(false);
    ui.tryWithoutLoginButton->setEnabled(false);
}

void ParserMaker::tryWithoutLogin() {
    session.initialize(&parser, &subscription, 0);
    loginWithoutCredentials = true;
    updateState();
    connect(&session, SIGNAL(receivedHtml(const QString&)), ui.loginTextEdit, SLOT(setPlainText(const QString&)));
    session.loginToForum();
    ui.tryLoginButton->setEnabled(false);
    ui.tryWithoutLoginButton->setEnabled(false);
}

void ParserMaker::loginFinished(ForumSubscription *sub, bool success) {
    if (loginWithoutCredentials) {
        if (success) {
            ui.loginResultLabel->setText("Search string found - login was successful. This is wrong.");
        } else {
            ui.loginResultLabel->setText("Search string not found, as expected.");
        }
    } else {
        if (success) {
            ui.loginResultLabel->setText("Search string found - login was successful, as expected.");
        } else {
            ui.loginResultLabel->setText("Search string not found. This is wrong.");
        }
    }

    disconnect(&session, SIGNAL(receivedHtml(const QString&)), ui.loginTextEdit, SLOT(setText(const QString&)));
    loginWithoutCredentials = false;

    ui.tryLoginButton->setEnabled(true);
    ui.tryWithoutLoginButton->setEnabled(true);
}

void ParserMaker::networkFailure(QString txt) {
    QMessageBox msgBox(this);
    msgBox.setText("Network Failure:\n" + txt);
    msgBox.exec();
    updateState();
}

void ParserMaker::helpClicked() {
    QUrl helpUrl;
    switch(ui.tabWidget->currentIndex()) {
    case 0:
        helpUrl.setUrl(protocol.baseURL() + "help/load_save.html");
        break;
    case 1:
        helpUrl.setUrl(protocol.baseURL() + "help/basics.html");
        break;
    case 2:
        helpUrl.setUrl(protocol.baseURL() + "help/login.html");
        break;
    case 3:
        helpUrl.setUrl(protocol.baseURL() + "help/grouplist.html");
        break;
    case 4:
        helpUrl.setUrl(protocol.baseURL() + "help/threadlist.html");
        break;
    case 5:
        helpUrl.setUrl(protocol.baseURL() + "help/messagelist.html");
        break;
    }
    QDesktopServices::openUrl(helpUrl);
}

void ParserMaker::getHttpAuthentication(ForumSubscription *fsub, QAuthenticator *authenticator) {
    CredentialsRequest cr;
    cr.credentialType = CredentialsRequest::SH_CREDENTIAL_HTTP;
    cr.subscription = fsub;
    CredentialsDialog *creds = new CredentialsDialog(this, &cr);
    creds->setModal(true);
    creds->exec();
    authenticator->setUser(cr.authenticator.user());
    authenticator->setPassword(cr.authenticator.password());
    creds->deleteLater();
}

void ParserMaker::dataMatched(int pos, QString data, PatternMatchType type) {
    // Horrible debugging, this still has bugs
    QString myData = ui.loginTextEdit->toPlainText().mid(pos, data.length());
    static int mismatches = 0;
    if(myData != data && mismatches < 5) {
        for(int i=0;i<data.length();i++) {
            if(myData[i] != data[i]) {
                int preDisplay = i-5;
                int postDisplay = i + 30;
                if(preDisplay < 0) preDisplay = 0;
                if(postDisplay > data.length()) postDisplay = data.length();
                qDebug() << Q_FUNC_INFO << "Data MisMatch at pos " << i << " pre " << preDisplay << ": \n" <<
                            myData.mid(preDisplay, postDisplay-preDisplay) <<
                            "\n != \n" << data.mid(preDisplay, postDisplay-preDisplay);
                qDebug() << Q_FUNC_INFO << "Problem char = " << myData.at(i) << " (" << (int) myData.at(i).toLatin1() <<  ") != "
                         << data.at(i) << "(" << (int) data.at(i).toLatin1() << ")";
                i = data.length();
                mismatches++;
            }
        }
    }

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

    loginEditorCursor.setPosition(loginEditorCursor.position() + data.length(),
                                  QTextCursor::KeepAnchor);
    QTextCharFormat fmt = loginEditorCursor.charFormat();
    fmt.setForeground(QBrush(color));
    loginEditorCursor.setCharFormat(fmt);
    loginEditorCursor.setPosition(loginEditorCursor.position(), QTextCursor::MoveAnchor);
}


void ParserMaker::dataMatchingStart(QString &html) {
    if (ui.loginTextEdit->toPlainText().length() == 0) {
        ui.loginTextEdit->setPlainText(html);
    }
    loginEditorCursor.setPosition(0, QTextCursor::MoveAnchor);
}

void ParserMaker::dataMatchingEnd() {
}
