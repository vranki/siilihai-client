#include "messageviewwidget.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/updateerror.h>
#include <siilihai/messageformatting.h>

#include <QDesktopServices>
#include <QDir>
#include <QDebug>

MessageViewWidget::MessageViewWidget(QWidget *parent) : QScrollArea(parent), webView(this), vbox(this), sourceView(false) {
    vbox.addWidget(&webView);
    setLayout(&vbox);
    webView.page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(&webView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(linkClicked(const QUrl &)));
    messageSelected(0);
}

MessageViewWidget::~MessageViewWidget() {
}

ForumMessage* MessageViewWidget::currentMessage() {
    return displayedMessage;
}

void MessageViewWidget::messageSelected(ForumMessage *msg) {
    displayedMessage = msg;
    disconnect(this, SLOT(currentMessageDeleted()));
    if (!msg) {
        webView.page()->setNetworkAccessManager(&nullNam);
        webView.load(QUrl("qrc:/data/blankmessage/index.html"));
    } else {
        qDebug() << Q_FUNC_INFO << "Selected message " << msg->toString() << "Unreads: " << msg->thread()->group()->subscription()->unreadCount()
                 << msg->thread()->group()->unreadCount() << msg->thread()->unreadCount();
        connect(msg, SIGNAL(destroyed()), this, SLOT(currentMessageDeleted()));
        if(msg->thread()->group()->subscription()->updateEngine()) {
            QNetworkAccessManager *nam = msg->thread()->group()->subscription()->updateEngine()->networkAccessManager();
            if(webView.page()->networkAccessManager()!=nam) {
                webView.page()->setNetworkAccessManager(nam); // Crashes??
            }
        }
        QString bodyToShow = msg->body();
        if(sourceView) {
            bodyToShow = "<div class=\"monospace\">" + msg->toString() + ":<br />" + MessageFormatting::replaceCharacters(bodyToShow) + "</div>";
            bodyToShow.replace("\n", "<br />");
        }
        QString styleHtml = "  <style type=\"text/css\">#siilihai-header {"
                "color: white;"
                "margin: 3px;"
                "padding: 3px 3%;"
                "background: url(\"qrc:/data/blankmessage/small_gradient.png\") 0% 0% repeat-x;"
                "}"
                "div.monospace { font-family: \"Fixed\",\"monospace\"; }"
                "div.quotecontent { background: #EEEEEE; margin: 5px; }"
                "div.quote { background: #EEEEEE; margin: 5px; }"
                "blockquote { background: #EEEEEE; margin: 5px; }"
                "td.quote { background: #EEEEEE; margin: 5px; }"
                "</style>";
        QString author = msg->author();
        QString lastchange = msg->lastchange();
        QString headerHtml = "<div id=\"siilihai-header\">" + MessageFormatting::sanitize(author) + ", "
                + MessageFormatting::sanitize(lastchange) + ":</div>";
        QString html = "<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" +
                styleHtml + "</head><body>" + headerHtml + bodyToShow + "</body>";

        QString baseUrl = msg->url();
        int i = baseUrl.lastIndexOf('/');
        if (i > 0) {
            baseUrl = baseUrl.left(i + 1);
        }
        webView.setContent(html.toUtf8(), QString("text/html"), QUrl(baseUrl));
        msg->setRead(true);
    }
    emit currentMessageChanged(msg);
}

void MessageViewWidget::linkClicked ( const QUrl & url) {
    QDesktopServices::openUrl(url);
}

bool MessageViewWidget::scrollDown() {
    return false;
}

void MessageViewWidget::displaySubscriptionErrors(ForumSubscription *sub)
{
    QString styleHtml = "  <style type=\"text/css\">"
                        "h2 { color: #880000}"
                        "pre { background: #EEEEEE}"
                        "</style>";

    QString bodyToShow = "<h1>" + sub->alias() + " update failed</h1>";
    foreach(UpdateError *ue, sub->errorList()) {
        bodyToShow += "<h2>" + ue->title() + "</h2>\n" + "<div>" + ue->description() + "</div>\n" + "<pre>" + MessageFormatting::replaceCharacters(ue->technicalData()) + "</pre>\n";
    }
    QString html = "<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" +
            styleHtml + "</head><body>" + bodyToShow + "</body>";
    webView.setContent(html.toUtf8(), QString("text/html"));
}

void MessageViewWidget::viewAsSource(bool src) {
    sourceView = src;
    messageSelected(currentMessage());
}

void MessageViewWidget::currentMessageDeleted() {
    messageSelected(0);
}
