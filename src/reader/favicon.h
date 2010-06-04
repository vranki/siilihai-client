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
#include <siilihai/forumsubscription.h>

class Favicon : public QObject {
    Q_OBJECT

public:
    Favicon(QObject *parent, ForumSubscription *s);
    void fetchIcon(const QUrl &url, const QPixmap &alt);
    void setReloading(bool rel, float progress = 0);
    virtual ~Favicon();
public slots:
    void replyReceived(QNetworkReply *reply);
    void update();
signals:
    void iconChanged(ForumSubscription *s, QIcon newIcon);
private:
    ForumSubscription *forum;
    bool reloading;
    QNetworkAccessManager nam;
    QPixmap currentpic;
    float currentProgress;
    float blinkAngle;
};

#endif /* FAVICON_H_ */
