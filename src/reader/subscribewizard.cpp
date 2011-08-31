#include "subscribewizard.h"

SubscribeWizard::SubscribeWizard(QWidget *parent, SiilihaiProtocol &proto, QString &baseUrl, QSettings &sett) :
    QWizard(parent), protocol(proto), settings(sett) {
    selectedParser = 0;
    setWizardStyle(QWizard::ModernStyle);
#ifndef Q_WS_HILDON
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/data/siilis_wizard_watermark.png"));
#endif
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    addPage(createIntroPage());
    addPage(createLoginPage());
    addPage(createVerifyPage());
    setWindowTitle("Subscribe to a forum");
    connect(&protocol, SIGNAL(listParsersFinished(QList <ForumParser*>)), this, SLOT(listParsersFinished(QList <ForumParser*>)));
    connect(subscribeForm.searchString, SIGNAL(textEdited(QString)), this, SLOT(updateParserList()));
    connect(this, SIGNAL(accepted()), this, SLOT(wizardAccepted()));
    connect(subscribeForm.displayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboItemChanged(int)));
    protocol.setBaseURL(baseUrl);
    progress = 0;
    parser = 0;
    protocol.listParsers();
    subscribeForm.forumList->addItem(QString("Downloading list of available forums..."));
    subscribeForm.pagePreview->hide();
    show();
}

SubscribeWizard::~SubscribeWizard() {
    qDeleteAll(allParsers);
    if(parser)
        parser->deleteLater();
}

QWizardPage *SubscribeWizard::createIntroPage() {
    QWizardPage *page = new QWizardPage;
#ifndef Q_WS_HILDON
    page->setTitle("Subscribe to a forum");
    page->setSubTitle("Please choose the forum you wish to subscribe to");
#endif

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    subscribeForm.setupUi(widget);
    layout->addWidget(widget);
    page->setLayout(layout);
    connect(subscribeForm.forumList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(forumClicked(QListWidgetItem*)));
    return page;
}

void SubscribeWizard::listParsersFinished(QList<ForumParser*> parsers) {
    allParsers = parsers;
    updateParserList();
}

void SubscribeWizard::updateParserList() {
    subscribeForm.forumList->clear();
    listWidgetItemForum.clear();
    foreach(ForumParser *parser, allParsers){
        bool displayParser = false;
        int parserType = parser->parser_type;
        int ci = subscribeForm.displayCombo->currentIndex();
        switch (ci) {
        case 0:
            if (parserType == 0)
                displayParser = true;
            break;
        case 1:
            if (parserType == 0 || parserType == 1)
                displayParser = true;
            break;
        case 2:
            if (parserType == 2)
                displayParser = true;
            break;
        case 3:
            displayParser = true;
            break;
        }
        if (displayParser) {
            if (parser->parser_name.contains(subscribeForm.searchString->text())
                || parser->forum_url.contains(subscribeForm.searchString->text())) {
                QListWidgetItem *item = new QListWidgetItem(subscribeForm.forumList);
                item->setText(parser->parser_name);
                item->setToolTip(parser->forum_url);
                subscribeForm.forumList->addItem(item);
                listWidgetItemForum[item] = parser;
            }
        }
    }
}

QWizardPage *SubscribeWizard::createLoginPage() {
    QWizardPage *page = new QWizardPage;
#ifndef Q_WS_HILDON
    page->setTitle("Forum account");

    page->setSubTitle(
            "If you have registered to the forum, you can enter your account credentials here");
#endif
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    subscribeForumLogin.setupUi(widget);
    layout->addWidget(widget);
    page->setLayout(layout);
    return page;
}

QWizardPage *SubscribeWizard::createVerifyPage() {
    QWizardPage *page = new QWizardPage;
#ifndef Q_WS_HILDON
    page->setTitle("Verify forum details");
    page->setSubTitle("Click Finish to subscribe to this forum");
#endif
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    subscribeForumVerify.setupUi(widget);
#ifdef Q_WS_HILDON
    subscribeForumVerify.forumPropertiesGroupBox->hide();
#endif
    layout->addWidget(widget);
    page->setLayout(layout);
    return page;
}

void SubscribeWizard::pageChanged(int id) {
    if (id == 0) {
        selectedParser = 0;
    } else if (id == 1) {
        if (allParsers.size() == 0 || subscribeForm.forumList->selectedItems().size() != 1) {
            back();
        } else {
            selectedParser = listWidgetItemForum[subscribeForm.forumList->selectedItems()[0]];
            connect(&protocol, SIGNAL(getParserFinished(ForumParser*)), this, SLOT(getParserFinished(ForumParser*)));

            protocol.getParser(selectedParser->id);
            progress = new QProgressDialog("Downloading parser definition..",
                                           "Cancel", 0, 3, this);
            progress->setWindowModality(Qt::WindowModal);
            progress->setValue(0);
        }
    } else if (id == 2) {
        if (subscribeForumLogin.accountGroupBox->isChecked()) {
            /*
			 progress = new QProgressDialog("Checking your credentials..",
			 "Cancel", 0, 3, this);
			 progress->setWindowModality(Qt::WindowModal);
			 progress->setValue(0);
			 */
        }
        subscribeForumVerify.forumName->setText(selectedParser->parser_name);
        subscribeForumVerify.forumUrl->setText(selectedParser->forum_url);
        QString typeString;
        if(selectedParser->parser_type == 0) {
            typeString = "Public";
        } else if(selectedParser->parser_type == 1) {
            typeString = "Private";
        } else {
            typeString = "Development";
        }
        subscribeForumVerify.forumType->setText(typeString);
    }
}

void SubscribeWizard::getParserFinished(ForumParser *fp) {
    disconnect(&protocol, SIGNAL(getParserFinished(ForumParser*)), this, SLOT(getParserFinished(ForumParser*)));
    QString warningLabel;
    if (fp->id >= 0) {
        parser = fp;
        subscribeForumLogin.accountGroupBox->setEnabled(parser->supportsLogin());
        if (fp->parser_status == 0) {
            warningLabel = "Note: This parser is new and has not been tested much.\n"
                           "Please report if it is working or not from the menu later.";
        } else if (fp->parser_status == 2) {
            warningLabel = "Warning: This parser has been reported as not working.\n"
                           "Please report if it is working or not from the menu later.";
        }
    } else {
        warningLabel = "Error: Unable to download parser definiton.\nCheck your network connection.";
        back();
    }
    if (progress) {
        progress->setValue(3);
        progress->deleteLater();
        progress = 0;
    }
    if (!warningLabel.isNull()) {
        QMessageBox msgBox(this);
        msgBox.setModal(true);
        msgBox.setText(warningLabel);
        msgBox.exec();
    }
}

void SubscribeWizard::wizardAccepted() {
    if(!parser->isSane()) {
        qDebug() << "Parsed not sane (yet), please hold on..";
        return;
    }
    QString user = QString::null;
    QString pass = QString::null;

    if (subscribeForumLogin.accountGroupBox->isChecked()) {
        user = subscribeForumLogin.usernameEdit->text();
        pass = subscribeForumLogin.passwordEdit->text();
    }

    ForumSubscription fs(this);
    fs.setParser(parser->id);
    fs.setAlias(subscribeForumVerify.forumName->text());
    fs.setUsername(user);
    fs.setPassword(pass);
    fs.setLatestThreads(settings.value("preferences/threads_per_group", 20).toInt());
    fs.setLatestMessages(settings.value("preferences/messages_per_thread", 20).toInt());

    Q_ASSERT(parser->isSane());
    emit(forumAdded(parser, &fs));
}

void SubscribeWizard::comboItemChanged(int id) {
    updateParserList();
}

void SubscribeWizard::forumClicked(QListWidgetItem* newItem) {
    ForumParser *fp = listWidgetItemForum.value(newItem);
    if(!fp) return;
    //subscribeForm.pagePreview->show(); sucks!
    //subscribeForm.pagePreview->load(QUrl(fp->forum_url));
}
