#include "subscribewizard.h"

#include <siilihai/parser/forumsubscriptionparsed.h>
#include <siilihai/tapatalk/forumsubscriptiontapatalk.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <QUrl>

SubscribeWizard::SubscribeWizard(QWidget *parent, SiilihaiProtocol &proto, QSettings &sett) :
    QWizard(parent), protocol(proto), settings(sett), newForum(0, true, ForumSubscription::FP_NONE),
    probe(this, proto) {
    // setWizardStyle(QWizard::ModernStyle);
    selectedForum = 0;
#ifndef Q_WS_HILDON
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/data/siilis_wizard_watermark.png"));
#endif
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    addPage(createIntroPage());
    addPage(createLoginPage());
    addPage(createVerifyPage());
    setWindowTitle("Subscribe to a forum");
    connect(&protocol, SIGNAL(listForumsFinished(QList <ForumSubscription*>)), this, SLOT(listForumsFinished(QList <ForumSubscription*>)));
    connect(subscribeForm.searchString, SIGNAL(textEdited(QString)), this, SLOT(updateForumList()));
    connect(this, SIGNAL(accepted()), this, SLOT(wizardAccepted()));
    connect(subscribeForm.displayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboItemChanged(int)));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));

    connect(subscribeForm.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(checkUrlValidity()));
    connect(subscribeForm.forumUrl, SIGNAL(textEdited(QString)), this, SLOT(checkUrlValidity()));
    connect(subscribeForm.forumList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(checkUrlValidity()));
    parser = 0;
    subscribeForm.forumList->addItem(QString("Downloading list of available forums..."));
    show();
    protocol.listForums();
    checkUrlValidity();
}

SubscribeWizard::~SubscribeWizard() {
    qDeleteAll(allForums);
    allForums.clear();
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

    subscribeForm.checkText->setVisible(false);
    subscribeForm.progressBar->setVisible(false);
    return page;
}

void SubscribeWizard::listForumsFinished(QList<ForumSubscription*> forums) {
    listWidgetItemForum.clear();
    qDeleteAll(allForums);
    allForums = forums;
    updateForumList();
}

void SubscribeWizard::updateForumList() {
    subscribeForm.forumList->clear();
    listWidgetItemForum.clear();
    foreach(ForumSubscription *forumIter, allForums){
        bool displayParser = true;
        /*
        int parserType = forumIter->parser_type;
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
        */
        if (displayParser) {
            if (forumIter->alias().contains(subscribeForm.searchString->text())
                    || forumIter->forumUrl().toString().contains(subscribeForm.searchString->text())) {
                QListWidgetItem *item = new QListWidgetItem(subscribeForm.forumList);
                item->setText(forumIter->alias());
                item->setToolTip(forumIter->forumUrl().toString());
                subscribeForm.forumList->addItem(item);
                listWidgetItemForum[item] = forumIter;
            }
        }
    }
}

QWizardPage *SubscribeWizard::createLoginPage() {
    QWizardPage *page = new QWizardPage;
#ifndef Q_WS_HILDON
    page->setTitle("Forum account");
    page->setSubTitle("If you have registered to the forum, you can enter your account credentials here");
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
    if (id == 0) { // Selection page
        selectedForum = 0;
        newForum.setForumId(0);
        newForum.setProvider(ForumSubscription::FP_NONE);
        subscribeForumLogin.accountGroupBox->setEnabled(true);
    } else if (id == 1) {
        if(subscribeForm.tabWidget->currentIndex()==0) { // Selected from list
            if(newForum.provider()==ForumSubscription::FP_NONE) {
                if (allForums.size() == 0 || subscribeForm.forumList->selectedItems().size() != 1) {
                } else {
                    selectedForum = listWidgetItemForum[subscribeForm.forumList->selectedItems()[0]];
                    connect(&protocol, SIGNAL(forumGot(ForumSubscription*)), this, SLOT(forumGot(ForumSubscription*)));
                    protocol.getForum(selectedForum->forumId());
                    button(QWizard::NextButton)->setEnabled(false);
                }
                back();
            }
        } else if(subscribeForm.tabWidget->currentIndex()==1) { // Custom
            QUrl forumUrl = QUrl(subscribeForm.forumUrl->text());
            subscribeForm.checkText->setVisible(true);
            if(newForum.provider()==ForumSubscription::FP_NONE) {
                if(forumUrl.isValid()) {
                    subscribeForm.progressBar->setVisible(true);
                    button(QWizard::NextButton)->setEnabled(false);
                    newForum.setProvider(ForumSubscription::FP_TAPATALK);
                    newForum.setForumUrl(forumUrl);
                    newForum.setAlias(forumUrl.toString());
                    connect(&probe, SIGNAL(probeResults(ForumSubscription*)), this, SLOT(probeResults(ForumSubscription*)));
                    probe.probeUrl(forumUrl);
                }
                back();
            } else {
                subscribeForm.forumUrl->setFocus();
                subscribeForm.checkText->setText("Invalid URI");
            }
        }
    } else if (id == 2) { // Verify page
        QString typeString;
        if(newForum.provider()==ForumSubscription::FP_PARSER) {
            if(parser.parser_type == 0) {
                typeString = "Public";
            } else if(parser.parser_type == 1) {
                typeString = "Private";
            } else {
                typeString = "Development";
            }
        } else if(newForum.provider()==ForumSubscription::FP_TAPATALK) {
            subscribeForumVerify.forumUrl->setText(newForum.forumUrl().toString());
            typeString = "TapaTalk";
        } else {
            Q_ASSERT(false);
        }
        subscribeForumVerify.forumType->setText(typeString);
    }
}

void SubscribeWizard::getParserFinished(ForumParser *fp) { // fp will be deleted after this
    disconnect(&protocol, SIGNAL(getParserFinished(ForumParser*)), this, SLOT(getParserFinished(ForumParser*)));
    QString warningLabel;
    if (fp && fp->id() >= 0) {
        parser = (*fp);
        Q_ASSERT(parser.id() == fp->id());
        subscribeForumLogin.accountGroupBox->setEnabled(parser.supportsLogin());
        if (fp->parser_status == 0) {
            warningLabel = "Note: This parser is new and has not been tested much.\n"
                    "Please report if it is working or not from the menu later.";
        } else if (fp->parser_status == 2) {
            warningLabel = "Warning: This parser has been reported as not working.\n"
                    "Please report if it is working or not from the menu later.";
        }
        next();
    } else {
        warningLabel = "Error: Unable to download parser definiton.\nCheck your network connection.";
        back();
    }
    if (!warningLabel.isNull()) {
        QMessageBox msgBox(this);
        msgBox.setModal(true);
        msgBox.setText(warningLabel);
        msgBox.exec();
    }
    button(QWizard::NextButton)->setEnabled(true);
}

void SubscribeWizard::wizardAccepted() {
    if(newForum.provider()==ForumSubscription::FP_PARSER && !parser.isSane()) {
        qDebug() << Q_FUNC_INFO << "Parsed not sane (yet), please hold on..";
        return;
    }

    QString user = QString::null;
    QString pass = QString::null;

    if (subscribeForumLogin.accountGroupBox->isChecked()) {
        user = subscribeForumLogin.usernameEdit->text();
        pass = subscribeForumLogin.passwordEdit->text();
    }
    ForumSubscription *fs = 0;
    if(newForum.provider()==ForumSubscription::FP_PARSER) {
        ForumSubscriptionParsed *fsParsed = new ForumSubscriptionParsed(this, true);
        fsParsed->setParser(parser.id());
        fs = fsParsed;
        fs->setForumUrl(QUrl(parser.forum_url));
        Q_ASSERT(parser.isSane());
    } else if(newForum.provider()==ForumSubscription::FP_TAPATALK) {
        ForumSubscriptionTapaTalk *fsTt = new ForumSubscriptionTapaTalk(this, true);
        fs = fsTt;
        fs->setForumUrl(newForum.forumUrl());
    }
    Q_ASSERT(fs);
    fs->setForumId(newForum.forumId());
    fs->setAlias(subscribeForumVerify.forumName->text());
    fs->setUsername(user);
    fs->setPassword(pass);
    fs->setAuthenticated(user.length() > 0);
    fs->setLatestThreads(settings.value("preferences/threads_per_group", 20).toInt());
    fs->setLatestMessages(settings.value("preferences/messages_per_thread", 20).toInt());

    emit(forumAdded(fs));
    deleteLater();
}

void SubscribeWizard::comboItemChanged(int id) {
    updateForumList();
}

void SubscribeWizard::forumClicked(QListWidgetItem* newItem) {
    ForumSubscription *fp = listWidgetItemForum.value(newItem);
    if(!fp) return;
    //subscribeForm.pagePreview->show(); sucks!
    //subscribeForm.pagePreview->load(QUrl(fp->forum_url));
}

void SubscribeWizard::newForumAdded(ForumSubscription *sub)
{
    disconnect(&protocol, SIGNAL(forumGot(ForumSubscription*)), this, SLOT(newForumAdded(ForumSubscription*)));
    if(sub) {
        Q_ASSERT(sub->forumId());
        newForum.setForumId(sub->forumId());
        newForum.setForumUrl(sub->forumUrl());
        newForum.setProvider(sub->provider());
        subscribeForumVerify.forumName->setText(sub->alias());
        subscribeForm.checkText->setText("Forum added");
        next();
    } else {
        subscribeForm.checkText->setText("Adding forum failed");
        newForum.setProvider(ForumSubscription::FP_NONE);
    }
    subscribeForm.progressBar->setVisible(false);
    button(QWizard::NextButton)->setEnabled(true);
}

void SubscribeWizard::probeResults(ForumSubscription *probedSub) {
    disconnect(&probe, SIGNAL(probeResults(ForumSubscription*)), this, SLOT(probeResults(ForumSubscription*)));
    if(!probedSub) {
        subscribeForm.checkText->setText("No supported forum found");
        subscribeForm.progressBar->setVisible(false);
        button(QWizard::NextButton)->setEnabled(true);
    } else {
        subscribeForm.checkText->setText("Found supported forum");
        newForum.setForumId(probedSub->forumId());
        newForum.setForumUrl(probedSub->forumUrl());
        newForum.setProvider(probedSub->provider());
        newForum.setAlias(probedSub->alias());
        if(newForum.alias().length() < 1) {
            newForum.setAlias(newForum.forumUrl().host());
            subscribeForumVerify.forumName->setText(newForum.alias());
        }
        subscribeForumVerify.forumName->setText(newForum.alias());
        subscribeForumVerify.forumUrl->setText(newForum.forumUrl().toString());
        if(newForum.forumId()) {
            subscribeForm.progressBar->setVisible(false);
            button(QWizard::NextButton)->setEnabled(true);
            next();
        } else {
            subscribeForm.checkText->setText("Adding forum to server..");
            connect(&protocol, SIGNAL(forumGot(ForumSubscription*)), this, SLOT(newForumAdded(ForumSubscription*)));
            protocol.addForum(&newForum);
        }
    }
}

void SubscribeWizard::forumGot(ForumSubscription *sub) {
    if(sub) {
        newForum.setForumId(sub->forumId());
        newForum.setForumUrl(sub->forumUrl());
        newForum.setProvider(sub->provider());
        newForum.setAlias(sub->alias());
        subscribeForumVerify.forumName->setText(newForum.alias());
        subscribeForumVerify.forumUrl->setText(newForum.forumUrl().toString());
        if(sub->provider() == ForumSubscription::FP_PARSER) {
            connect(&protocol, SIGNAL(getParserFinished(ForumParser*)), this, SLOT(getParserFinished(ForumParser*)));
            protocol.getParser(newForum.forumId());
        } else {
            probeResults(sub);
        }
    } else {
        QMessageBox msgBox(this);
        msgBox.setModal(true);
        msgBox.setText("Unable to download forum info");
        msgBox.exec();
    }
}

void SubscribeWizard::checkUrlValidity() {
    if(subscribeForm.tabWidget->currentIndex()==0) { // Selected from list
        button(QWizard::NextButton)->setEnabled(subscribeForm.forumList->currentItem());
    } else if(subscribeForm.tabWidget->currentIndex()==1){ // Custom
        QUrl forumUrl = QUrl(subscribeForm.forumUrl->text());
        button(QWizard::NextButton)->setEnabled(forumUrl.isValid());
    }
}
