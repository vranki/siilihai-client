#include "messageviewwidget.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/messageformatting.h>

#include <QDesktopServices>
#include <QDir>
#include <QDebug>

MessageViewWidget::MessageViewWidget(QWidget *parent) : QScrollArea(parent), webView(this), vbox(this) {
    vbox.addWidget(&webView);
    setLayout(&vbox);
    webView.page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(&webView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(linkClicked(const QUrl &)));
    messageSelected(0);
    viewMode = 0;
}

MessageViewWidget::~MessageViewWidget() {
}

ForumMessage* MessageViewWidget::currentMessage() {
    return displayedMessage;
}

void MessageViewWidget::messageSelected(ForumMessage *msg) {
    displayedMessage = msg;
    if (!msg) {
        webView.page()->setNetworkAccessManager(&nullNam);
        webView.load(QUrl("qrc:/data/blankmessage/index.html"));
    } else {
        qDebug() << Q_FUNC_INFO << "Selected message " << msg->toString() << "Unreads: " << msg->thread()->group()->subscription()->unreadCount()
                 << msg->thread()->group()->unreadCount() << msg->thread()->unreadCount();
        if(msg->thread()->group()->subscription()->updateEngine()) {
            QNetworkAccessManager *nam = msg->thread()->group()->subscription()->updateEngine()->networkAccessManager();
            if(webView.page()->networkAccessManager()!=nam) {
                webView.page()->setNetworkAccessManager(nam); // Crashes??
            }
        }
        // This looks like a big hack but works pretty well :-)
        QString bodyToShow = msg->body();
        if(viewMode == VIEW_TEXT) {
            bodyToShow.replace("<br>", "\n");
            bodyToShow.replace("<br/>", "\n");
            bodyToShow.replace("<br />", "\n");
            bodyToShow.replace("<p>", "\n<p>");
            bodyToShow.replace("<div", "\n<div");
            bodyToShow.replace("<a ", "\n<a ");
            bodyToShow.replace("<img ", "\n<img ");
            bodyToShow = "<div class=\"monospace\">" + MessageFormatting::stripHtml(bodyToShow) + "</div>";
        } else if(viewMode == VIEW_SOURCE) {
            bodyToShow = "<div class=\"monospace\">" +
                    msg->toString() + ":<br />" + MessageFormatting::replaceCharacters(bodyToShow) + "</div>";
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

void MessageViewWidget::viewAsHTML() {
    viewMode = VIEW_HTML;
    messageSelected(currentMessage());
}

void MessageViewWidget::viewAsText() {
    viewMode = VIEW_TEXT;
    messageSelected(currentMessage());
}

void MessageViewWidget::viewAsSource() {
    viewMode = VIEW_SOURCE;
    messageSelected(currentMessage());
}
