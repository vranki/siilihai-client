#include "subscribewizard.h"

#include <siilihai/parser/forumsubscriptionparsed.h>
#include <siilihai/tapatalk/forumsubscriptiontapatalk.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/subscriptionmanagement.h>
#include <QUrl>
#include <QAbstractButton>
#include <QMessageBox>

SubscribeWizard::SubscribeWizard(QWidget *parent, SubscriptionManagement *subscriptionManagement) :
    QWizard(parent), m_subscriptionManagement(subscriptionManagement) {
    selectedForum = 0;
#ifndef Q_WS_HILDON
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/data/siilis_wizard_watermark.png"));
#endif
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    addPage(createIntroPage());
    addPage(createLoginPage());
    addPage(createVerifyPage());
    setWindowTitle("Subscribe to a forum");
    connect(subscribeForm.searchString, SIGNAL(textEdited(QString)), m_subscriptionManagement, SLOT(setForumFilter(QString)));
    connect(this, SIGNAL(accepted()), this, SLOT(wizardAccepted()));
    connect(subscribeForm.displayCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboItemChanged(int)));
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));

    connect(subscribeForm.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(checkUrlValidity()));
    connect(subscribeForm.forumUrl, SIGNAL(textEdited(QString)), this, SLOT(checkUrlValidity()));
    connect(subscribeForm.forumList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(checkUrlValidity()));
    subscribeForm.forumList->addItem(QString("Downloading list of available forums..."));
    show();

    connect(m_subscriptionManagement, SIGNAL(forumListChanged()), this, SLOT(forumListChanged()));
    connect(m_subscriptionManagement, SIGNAL(newForumChanged(ForumSubscription*)), this, SLOT(newForumChanged(ForumSubscription*)));
    connect(m_subscriptionManagement, SIGNAL(showError(QString)), subscribeForm.checkText, SLOT(setText(QString)));
    m_subscriptionManagement->listForums();

    checkUrlValidity();
}

SubscribeWizard::~SubscribeWizard() {
}

void SubscribeWizard::forumListChanged() {
    listWidgetItemForum.clear();
    updateForumList();
}

void SubscribeWizard::updateForumList() {
    subscribeForm.forumList->clear();
    listWidgetItemForum.clear();
    for(QObject *forumObj : m_subscriptionManagement->forumList()){
        ForumSubscription *forumIter = qobject_cast<ForumSubscription*>(forumObj);
        QListWidgetItem *item = new QListWidgetItem(subscribeForm.forumList);
        item->setText(forumIter->alias());
        item->setToolTip(forumIter->forumUrl().toString());
        subscribeForm.forumList->addItem(item);
        listWidgetItemForum[item] = forumIter;
    }
}

void SubscribeWizard::newForumChanged(ForumSubscription *sub)
{
    subscribeForm.progressBar->setVisible(false);
    button(QWizard::NextButton)->setEnabled(sub);
    if(sub) {
        Q_ASSERT(sub->id());
        subscribeForumVerify.forumName->setText(sub->alias());
        next();
    }
}

void SubscribeWizard::pageChanged(int id) {
    if (id == 0) { // Selection page
        selectedForum = 0;
        subscribeForumLogin.accountGroupBox->setEnabled(true);
        m_subscriptionManagement->resetNewForum();
    } else if (id == 1) { // Credentals page
        if(!m_subscriptionManagement->newForum()) {
            if(subscribeForm.tabWidget->currentIndex()==0) { // Selected from list
                if (m_subscriptionManagement->forumList().size() == 0 || subscribeForm.forumList->selectedItems().size() != 1) {
                } else {
                    selectedForum = listWidgetItemForum[subscribeForm.forumList->selectedItems()[0]];
                    m_subscriptionManagement->getForum(selectedForum->id());
                    button(QWizard::NextButton)->setEnabled(false);
                }
            } else if(subscribeForm.tabWidget->currentIndex()==1) { // Custom
                QUrl forumUrl = QUrl(subscribeForm.forumUrl->text());
                subscribeForm.checkText->setVisible(true);
                if(forumUrl.isValid()) {
                    subscribeForm.progressBar->setVisible(true);
                    button(QWizard::NextButton)->setEnabled(false);
                    m_subscriptionManagement->getForum(forumUrl);
                } else {
                    subscribeForm.forumUrl->setFocus();
                    subscribeForm.checkText->setText("Invalid URL");
                }
            }
            // Go back, if we don't have a new forum available.
            if(!m_subscriptionManagement->newForum()) {
                back();
            }
        }
    } else if (id == 2) { // Verify page
        QString typeString = m_subscriptionManagement->newForum()->providerName();
        if(m_subscriptionManagement->newForum()->provider()!=ForumSubscription::FP_PARSER) {
            subscribeForumVerify.forumUrl->setText(m_subscriptionManagement->newForum()->forumUrl().toString());
        }
        subscribeForumVerify.forumType->setText(typeString);
    }
}

void SubscribeWizard::wizardAccepted() {
    QString user = QString::null;
    QString pass = QString::null;

    if (subscribeForumLogin.accountGroupBox->isChecked()) {
        user = subscribeForumLogin.usernameEdit->text();
        pass = subscribeForumLogin.passwordEdit->text();
    }

    m_subscriptionManagement->subscribeThisForum(user, pass);

    deleteLater();
}

void SubscribeWizard::comboItemChanged(int id) {
    Q_UNUSED(id);
    updateForumList();
}

void SubscribeWizard::forumClicked(QListWidgetItem* newItem) {
    ForumSubscription *fp = listWidgetItemForum.value(newItem);
    if(!fp) return;
}

/*
void SubscribeWizard::probeResults(ForumSubscription *probedSub) {
    disconnect(&probe, SIGNAL(probeResults(ForumSubscription*)), this, SLOT(probeResults(ForumSubscription*)));
    if(!probedSub) {
        subscribeForm.checkText->setText("No supported forum found");
        subscribeForm.progressBar->setVisible(false);
        button(QWizard::NextButton)->setEnabled(true);
    } else {
        subscribeForm.checkText->setText("Found supported forum");
        newForum.setId(probedSub->id());
        newForum.setForumUrl(probedSub->forumUrl());
        newForum.setProvider(probedSub->provider());
        newForum.setAlias(probedSub->alias());
        if(newForum.alias().length() < 1) {
            newForum.setAlias(newForum.forumUrl().host());
            subscribeForumVerify.forumName->setText(newForum.alias());
        }
        subscribeForumVerify.forumName->setText(newForum.alias());
        subscribeForumVerify.forumUrl->setText(newForum.forumUrl().toString());
        if(newForum.id()) {
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
*/

void SubscribeWizard::checkUrlValidity() {
    if(subscribeForm.tabWidget->currentIndex()==0) { // Selected from list
        button(QWizard::NextButton)->setEnabled(subscribeForm.forumList->currentItem());
    } else if(subscribeForm.tabWidget->currentIndex()==1){ // Custom
        QUrl forumUrl = QUrl(subscribeForm.forumUrl->text());
        button(QWizard::NextButton)->setEnabled(forumUrl.isValid());
    }
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
