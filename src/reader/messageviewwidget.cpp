#include "messageviewwidget.h"

MessageViewWidget::MessageViewWidget(QWidget *parent) : QScrollArea(parent), webView(this), vbox(this) {
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
    if (!msg) {
#ifdef STORE_FILES_IN_APP_DIR
        webView.load(QUrl("file://" + QDir::currentPath() + "/data/blankmessage/index.html"));
#else
        webView.load(QUrl("file:///usr/share/siilihai/blankmessage/index.html"));
#endif
        return;
    }
    qDebug() << Q_FUNC_INFO << msg->toString() << "ordernum: " << msg->ordernum();


    QString bodyToShow = msg->body();
    QString styleHtml = "  <style type=\"text/css\">#siilihai-header {"
                        "color: white;"
                        "margin: 3px;"
                        "padding: 3px 3%;"
                        "background: url(\"file:///usr/share/siilihai/blankmessage/small_gradient.png\") 0% 0% repeat-x;"
                        "}"
                        "</style>";
    QString author = msg->author();
    QString lastchange = msg->lastchange();
    QString headerHtml = "<div id=\"siilihai-header\">" + MessageFormatting::sanitize(author) + ", "
                         + MessageFormatting::sanitize(lastchange) + ":</div>";
    QString
            html =
            "<html><head><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" +
            styleHtml + "</head>"
            "<body>" + headerHtml + bodyToShow + "</body>";

    QString baseUrl = msg->url();
    int i = baseUrl.lastIndexOf('/');
    if (i > 0) {
        baseUrl = baseUrl.left(i + 1);
    }
    webView.setContent(html.toUtf8(), QString("text/html"), QUrl(baseUrl));
}

void MessageViewWidget::linkClicked ( const QUrl & url) {
    QDesktopServices::openUrl(url);
}

bool MessageViewWidget::scrollDown() {
    return false;
}
