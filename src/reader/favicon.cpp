#include "favicon.h"
#include <siilihai/forumthread.h>
#include <siilihai/forumsubscription.h>

Favicon::Favicon(QObject *parent, ForumSubscription *sub) : QObject(parent) {
    subscription = sub;
    currentProgress = 0;
    reloading = false;
    connect(sub, SIGNAL(changed(ForumSubscription*)), this, SLOT(subscriptionChanged(ForumSubscription*)));
    connect(&blinkTimer, SIGNAL(timeout()), this, SLOT(update()));
    blinkTimer.setInterval(100);
    blinkTimer.setSingleShot(false);
    subscriptionChanged(sub);
}

Favicon::~Favicon() {
}

void Favicon::subscriptionChanged(ForumSubscription *sub) {
    if(subscription->parserEngine()) {
        connect(subscription->parserEngine(),
                SIGNAL(stateChanged(ParserEngine *, ParserEngine::ParserEngineState, ParserEngine::ParserEngineState)),
                this,
                SLOT(engineStateChanged(ParserEngine *, ParserEngine::ParserEngineState)));
        engineStateChanged(subscription->parserEngine(), subscription->parserEngine()->state());
    } else {
    }
}

void Favicon::fetchIcon(const QUrl &url, const QPixmap &alt) {
    //    qDebug() << Q_FUNC_INFO << "Fetching icon " << url.toString() << " for " << engine->subscription()->toString();
    currentpic = alt;
    blinkAngle = 0;
    QNetworkRequest req(url);
    connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyReceived(QNetworkReply*)), Qt::UniqueConnection);
    nam.get(req);
    emit iconChanged(subscription, currentpic);
}

void Favicon::replyReceived(QNetworkReply *reply) {
    disconnect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyReceived(QNetworkReply*)));
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();
        QPixmap iconPixmap;
        iconPixmap.loadFromData(bytes);
        if(!iconPixmap.isNull()) {
            currentpic = iconPixmap;
            emit iconChanged(subscription, currentpic);
        }
    }
    reply->deleteLater();
}

void Favicon::update() {
    QPixmap outPic(currentpic);
    blinkAngle -= 0.05;

    QPainter painter(&outPic);

    QRect rect(0, 0, outPic.width(), outPic.height());

    painter.setPen(QColor(255, 255, 255, 64));
    painter.setBrush(QColor(255, 255, 255, 128));
    painter.drawPie(rect, blinkAngle * 5760, 1500);
    painter.setPen(QColor(0, 0, 0, 64));
    painter.setBrush(QColor(0, 0, 0, 128));
    painter.drawPie(rect, blinkAngle * 5760 - (5760/2), 1500);

    emit iconChanged(subscription, QIcon(outPic));
}

void Favicon::engineStateChanged(ParserEngine *engine, ParserEngine::ParserEngineState newState) {
    if(newState==ParserEngine::PES_UPDATING || newState==ParserEngine::PES_UPDATING_PARSER) {
        update();
        blinkTimer.start();
        //        emit iconChanged(subscription, QIcon(":data/view-refresh.png"));
    } else if(newState==ParserEngine::PES_IDLE) {
        blinkAngle = 0;
        blinkTimer.stop();
        emit iconChanged(subscription, currentpic);
    } else if(newState==ParserEngine::PES_ERROR) {
        blinkAngle = 0;
        blinkTimer.stop();

        QPixmap outPic(currentpic);
        QPainter painter(&outPic);
        painter.drawPixmap(0,0,outPic.width(),outPic.height(), QPixmap(":data/dialog-error.png"));

        emit iconChanged(subscription, QIcon(outPic));
    }
}
/*
void Favicon::setReloading(bool rel, float progress) {
    return;
    if (rel != reloading || currentProgress != progress) {
        currentProgress = progress;
        if (currentProgress > 1)
            currentProgress = 1;

        reloading = rel;
        update();
    }
}
*/
