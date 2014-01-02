#ifndef MESSAGEVIEWWIDGET_H_
#define MESSAGEVIEWWIDGET_H_

#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

#include <QWebView>
#include <QVBoxLayout>
#include <QUrl>
#include <QScrollArea>

class ForumMessage;

class MessageViewWidget : public QScrollArea {
    Q_OBJECT

public:
    MessageViewWidget(QWidget *parent);
    virtual ~MessageViewWidget();
    ForumMessage* currentMessage();
    bool scrollDown(); // Scrolls down if possible and returns true, if it did so.
signals:
    void currentMessageChanged(ForumMessage *msg);
public slots:
    void messageSelected(ForumMessage *msg);
    void linkClicked ( const QUrl & url);
    void viewAsSource(bool src);
private slots:
    void currentMessageDeleted();
private:
    QWebView webView;
    QVBoxLayout vbox;
    ForumMessage *displayedMessage;
    bool sourceView;
    QNetworkAccessManager nullNam; // Used when no msg is displayed
};

#endif /* MESSAGEVIEWWIDGET_H_ */
