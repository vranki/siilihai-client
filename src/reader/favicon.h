#ifndef FAVICON_H_
#define FAVICON_H_
#include <QIcon>
#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QByteArray>
#include <QDebug>
#include <QRect>
#include <QPainter>
#include <QTimer>
#include <cmath>
#include <siilihai/parserengine.h>

class Favicon : public QObject {
    Q_OBJECT

public:
    Favicon(QObject *parent, ForumSubscription *fs);
    void fetchIcon(const QUrl &url, const QPixmap &alt);
    //void setReloading(bool rel, float progress = 0);
    virtual ~Favicon();
public slots:
    void replyReceived(QNetworkReply *reply);
    void update();
    void subscriptionChanged(ForumSubscription *sub);
    void engineStateChanged(ParserEngine::ParserEngineState newState);
//    void engineStatusChanged(ForumSubscription* fs,bool reloading,float progress);
signals:
    void iconChanged(ForumSubscription *e, QIcon newIcon);
private:
    ForumSubscription *subscription;
    bool reloading;
    QNetworkAccessManager nam;
    QPixmap currentpic;
    float currentProgress;
    float blinkAngle;
    QTimer blinkTimer;
};

#endif /* FAVICON_H_ */
