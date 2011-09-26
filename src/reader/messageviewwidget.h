#ifndef MESSAGEVIEWWIDGET_H_
#define MESSAGEVIEWWIDGET_H_
#include <QObject>
#include <QScrollArea>
#include <QWebView>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QDir>

class ForumMessage;

class MessageViewWidget : public QScrollArea {
    Q_OBJECT

public:
    MessageViewWidget(QWidget *parent);
    virtual ~MessageViewWidget();
    ForumMessage* currentMessage();
    bool scrollDown(); // Scrolls down if possible and returns true, if it did so.
public slots:
    void messageSelected(ForumMessage *msg);
    void linkClicked ( const QUrl & url);
    void viewAsHTML();
    void viewAsText();
    void viewAsSource();
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
