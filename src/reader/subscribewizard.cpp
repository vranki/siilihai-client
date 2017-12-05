#include "subscribewizard.h"

#include <siilihai/parser/forumsubscriptionparsed.h>
#include <siilihai/tapatalk/forumsubscriptiontapatalk.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/subscriptionmanagement.h>
#include <QUrl>
#include <QAbstractButton>
#include <QMessageBox>

SubscribeWizard::SubscribeWizard(QWidget *parent, SubscriptionManagement *subscriptionManagement) :
    QWizard(parent)
  , m_subscriptionManagement(subscriptionManagement)
  , selectedForum(nullptr) {

    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/data/siilis_wizard_watermark.png"));
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
    connect(m_subscriptionManagement, SIGNAL(probeInProgressChanged(bool)), this, SLOT(probeInProgressChanged(bool)));
    m_subscriptionManagement->listForums();

    checkUrlValidity();
}

SubscribeWizard::~SubscribeWizard() { }

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
    if(sub) {
        Q_ASSERT(sub->id());
        subscribeForumVerify.forumName->setText(sub->alias());
        next();
    } else {
        selectedForum = nullptr;
    }
}

void SubscribeWizard::pageChanged(int id) {
    if (id == 0) { // Selection page
        selectedForum = nullptr;
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
                if(forumUrl.isValid()) {
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

void SubscribeWizard::checkUrlValidity() {
    if(subscribeForm.tabWidget->currentIndex()==0) { // Selected from list
        button(QWizard::NextButton)->setEnabled(subscribeForm.forumList->currentItem());
    } else if(subscribeForm.tabWidget->currentIndex()==1){ // Custom
        QUrl forumUrl = QUrl(subscribeForm.forumUrl->text());
        button(QWizard::NextButton)->setEnabled(forumUrl.isValid());
    }
}

void SubscribeWizard::probeInProgressChanged(bool pib)
{
    qDebug() << Q_FUNC_INFO << pib;
    subscribeForm.progressBar->setVisible(pib);
    button(QWizard::NextButton)->setEnabled(!pib);
    subscribeForm.checkText->setVisible(!pib);
}

QWizardPage *SubscribeWizard::createIntroPage() {
    QWizardPage *page = new QWizardPage;
    page->setTitle("Subscribe to a forum");
    page->setSubTitle("Please choose the forum you wish to subscribe to");
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    subscribeForm.setupUi(widget);
    layout->addWidget(widget);
    page->setLayout(layout);
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
