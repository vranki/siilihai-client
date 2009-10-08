#include "favicon.h"

Favicon::Favicon(QObject *parent, int forumid) :
	QObject(parent) {
	forum = forumid;
}

Favicon::~Favicon() {
}

void Favicon::fetchIcon(const QUrl &url, const QPixmap &alt) {
	alternative = alt;
	QNetworkRequest req(url);
	connect(&nam, SIGNAL(finished(QNetworkReply*)), this,
			SLOT(replyReceived(QNetworkReply*)));
	nam.get(req);
}

void Favicon::replyReceived(QNetworkReply *reply) {
	disconnect(&nam, SIGNAL(finished(QNetworkReply*)), this,
			SLOT(replyReceived(QNetworkReply*)));
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray bytes = reply->readAll();
		qDebug() << bytes;
		downloaded.loadFromData(bytes);
		emit iconChanged(forum, QIcon(downloaded));
	} else {
		emit iconChanged(forum, QIcon(alternative));
	}
	reply->deleteLater();
}

void Favicon::update() {
	if (reloading) {
		emit iconChanged(forum, QIcon(":/data/view-refresh.png"));
	} else {
		if (downloaded.isNull()) {
			emit iconChanged(forum, QIcon(alternative));
		} else {
			emit iconChanged(forum, QIcon(downloaded));
		}
	}
}

void Favicon::setReloading(bool rel) {
	if (rel != reloading) {
		reloading = rel;
		update();
	} else {
		reloading = rel;
	}
}
