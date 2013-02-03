#ifndef MESSAGEVIEWWIDGET_H_
#define MESSAGEVIEWWIDGET_H_
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
    void viewAsHTML();
    void viewAsText();
    void viewAsSource();
private slots:
    void currentMessageDeleted();
private:
    const static int VIEW_HTML=0;
    const static int VIEW_TEXT=1;
    const static int VIEW_SOURCE=2;
    QWebView webView;
    QVBoxLayout vbox;
    ForumMessage *displayedMessage;
    int viewMode;
    QNetworkAccessManager nullNam; // Used when no msg is displayed
};

#endif /* MESSAGEVIEWWIDGET_H_ */
