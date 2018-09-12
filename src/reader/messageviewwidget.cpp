#include "messageviewwidget.h"
#include <siilihai/forumdata/forummessage.h>
#include <siilihai/parser/parserengine.h>
#include <siilihai/forumdata/forumthread.h>
#include <siilihai/forumdata/forumgroup.h>
#include <siilihai/forumdata/forumsubscription.h>
#include <siilihai/forumdata/updateerror.h>
#include <siilihai/messageformatting.h>
#include "linkhandlingwebpage.h"

#include <QDesktopServices>
#include <QDir>
#include <QDebug>

MessageViewWidget::MessageViewWidget(QWidget *parent) :
    QScrollArea(parent),
    vbox(this),
    sourceView(false) {
    webView.setPage(new LinkHandlingWebPage(&webView));
    vbox.addWidget(&webView);
    setLayout(&vbox);
    messageSelected(nullptr);
}

MessageViewWidget::~MessageViewWidget() { }

ForumMessage* MessageViewWidget::currentMessage() {
    return displayedMessage;
}

void MessageViewWidget::messageSelected(ForumMessage *msg) {
    displayedMessage = msg;
    disconnect(this, SLOT(currentMessageDeleted()));
    if (!msg) {
        //webView.page()->setNetworkAccessManager(&nullNam);
        webView.page()->load(QUrl("qrc:///data/blankmessage/index.html"));
    } else {
        connect(msg, SIGNAL(destroyed()), this, SLOT(currentMessageDeleted()));
        if(msg->thread()->group()->subscription()->updateEngine()) {
            /*
            QNetworkAccessManager *nam = msg->thread()->group()->subscription()->updateEngine()->networkAccessManager();
            if(webView.page()->networkAccessManager()!=nam) {
                webView.page()->setNetworkAccessManager(nam); // Crashes??
            }
            */
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
        QString lastChange = msg->lastChange();
        QString headerHtml = "<div id=\"siilihai-header\">" + MessageFormatting::sanitize(author) + ", "
                + MessageFormatting::sanitize(lastChange) + ":</div>";
        /*
        QString html = "<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" +
                styleHtml + "</head><body>" + headerHtml + bodyToShow + "</body>";
                */
        QString html = "<html><head>"
                + styleHtml
                + "</head><body>"
                + headerHtml
                + bodyToShow
                + "</body>";

        QString baseUrl = msg->url();
        int i = baseUrl.lastIndexOf('/');
        if (i > 0) {
            baseUrl = baseUrl.left(i + 1);
        }
        webView.page()->setHtml(html, QUrl(baseUrl));

        msg->setRead(true);
    }
    emit currentMessageChanged(msg);
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
    for(UpdateError *ue : sub->errorList()) {
        bodyToShow += "<h2>" + ue->title() + "</h2>\n" + "<div>" + ue->description() + "</div>\n" + "<pre>" + MessageFormatting::replaceCharacters(ue->technicalData()) + "</pre>\n";
    }
    /*
    QString html = "<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" +
            styleHtml + "</head><body>" + bodyToShow + "</body>";
            */
    QString html = "<html><head>"
            + styleHtml
            + "</head><body>"
            + bodyToShow
            + "</body>";
    webView.page()->setHtml(html);
}

void MessageViewWidget::viewAsSource(bool src) {
    sourceView = src;
    messageSelected(currentMessage());
}

void MessageViewWidget::currentMessageDeleted() {
    messageSelected(nullptr);
}
