#ifndef LINKHANDLINGWEBPAGE_H
#define LINKHANDLINGWEBPAGE_H

#include <QObject>
#include <QWebEnginePage>

/**
 * @brief The LinkHandlingWebPage class is a QWebEnginePage
 * which opens any clicked links in external browser.
 */
class LinkHandlingWebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    LinkHandlingWebPage(QObject* parent = 0);
    virtual bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame);
};

#endif // LINKHANDLINGWEBPAGE_H
