/*
 * favicon.h
 *
 *  Created on: Oct 8, 2009
 *      Author: vranki
 */

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

class Favicon : public QObject {
	Q_OBJECT

public:
	Favicon(QObject *parent, int forumid);
	void fetchIcon(const QUrl &url, const QPixmap &alt);
	void setReloading(bool rel);
	void update();
	virtual ~Favicon();
public slots:
	void replyReceived(QNetworkReply *reply);
signals:
	void iconChanged(int forumid, QIcon newIcon);
private:
	int forum;
	bool reloading;
	QNetworkAccessManager nam;
	QPixmap alternative, downloaded;
};

#endif /* FAVICON_H_ */
