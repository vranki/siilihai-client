#ifndef MESSAGEVIEWWIDGET_H_
#define MESSAGEVIEWWIDGET_H_

#include <QWebEngineView>
#include <QVBoxLayout>
#include <QUrl>
#include <QScrollArea>

class ForumMessage;
class ForumSubscription;

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
    void displaySubscriptionErrors(ForumSubscription *sub);
    void messageSelected(ForumMessage *msg);
    void linkClicked ( const QUrl & url);
    void viewAsSource(bool src);
private slots:
    void currentMessageDeleted();
private:
    QWebEngineView webView;
    QVBoxLayout vbox;
    ForumMessage *displayedMessage;
    bool sourceView;
    QNetworkAccessManager nullNam; // Used when no msg is displayed
};

#endif /* MESSAGEVIEWWIDGET_H_ */
