#include "previewpage.h"

#include <QDesktopServices>

bool PreviewPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType, bool)
{
    if (url.scheme() == QString("qrc")) //只有以“qrc”开头的链接才使用webview打开
        return true;
    QDesktopServices::openUrl(url); //其他链接使用系统浏览器打开
    return false;
}
