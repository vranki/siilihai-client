#include "linkhandlingwebpage.h"
#include <QDesktopServices>

LinkHandlingWebPage::LinkHandlingWebPage(QObject* parent) : QWebEnginePage(parent)
{ }

bool LinkHandlingWebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame);
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        QDesktopServices::openUrl(url);
    }
    return false;
}
