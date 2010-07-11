#include "favicon.h"

Favicon::Favicon(QObject *parent, ParserEngine *pe) :
	QObject(parent) {
    engine = pe;
    currentProgress = 0;
    reloading = false;
    connect(engine, SIGNAL(statusChanged(ForumSubscription*,bool,float)),
            this, SLOT(engineStatusChanged(ForumSubscription*,bool,float)));
    Q_ASSERT(engine);
}

Favicon::~Favicon() {
}

void Favicon::fetchIcon(const QUrl &url, const QPixmap &alt) {
//    qDebug() << Q_FUNC_INFO << "Fetching icon " << url.toString() << " for " << engine->subscription()->toString();
    currentpic = QIcon(alt);
    blinkAngle = 0;
    QNetworkRequest req(url);
    connect(&nam, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(replyReceived(QNetworkReply*)));
    nam.get(req);
    emit iconChanged(engine, currentpic);
    update();
}

void Favicon::replyReceived(QNetworkReply *reply) {
    disconnect(&nam, SIGNAL(finished(QNetworkReply*)), this,
               SLOT(replyReceived(QNetworkReply*)));
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();
        QPixmap iconPixmap;
        iconPixmap.loadFromData(bytes);
        currentpic = QIcon(iconPixmap);
        emit iconChanged(engine, currentpic);
    }
    reply->deleteLater();
}

void Favicon::update() {
    return;
    /*
    QPixmap outPic(currentpic);
    if (reloading) {
        // Slower FPS for Maemo
        outPic.scroll(sinf(blinkAngle)*5, 0, outPic.rect());
#ifdef Q_WS_HILDON
        QTimer::singleShot(120, this, SLOT(update()));
        blinkAngle += 0.1;
#else
        QTimer::singleShot(25, this, SLOT(update()));
        blinkAngle += 0.05;
#endif
        */
        /*
        QPainter painter(&outPic);
        painter.setPen(QColor(255, 255, 255, 64));
        painter.setBrush(QColor(255, 255, 255, 128));
        QRect rect(0, 0, outPic.width(), outPic.height());
        painter.drawPie(rect, blinkAngle * 5760, 1000);
        painter.setPen(QColor(0, 0, 0, 64));
        painter.setBrush(QColor(0, 0, 0, 128));
        painter.drawPie(rect, blinkAngle * 5760 - (5760/2), 1000);
*/
/*
		painter.setBrush(QColor(0, 0, 0, 200));

		if (currentProgress >= 0) {
			rect.setRect(0, outPic.height() - 8, outPic.width(), 8);
			painter.drawRects(&rect, 1);

			float barLength = (float) outPic.width() * currentProgress;
			painter.setBrush(QColor(0, 0, 128, 128));
			rect.setRect(0, outPic.height() - 8, barLength, 8);
			painter.drawRects(&rect, 1);
		}
		*/
  //      painter.end();
   // }
   // emit iconChanged(engine, QIcon(outPic));
}

void Favicon::engineStatusChanged(ForumSubscription* fs,bool reloading,float progress) {
    if(reloading) {
        emit iconChanged(engine, QIcon(":data/view-refresh.png"));
    } else {
        emit iconChanged(engine, currentpic);
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
