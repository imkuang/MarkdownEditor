#include "document.h"

void Document::setText(const QString &text)
{
    if (text == contentText)  //在文本内容改变时将改变后的文本赋值给contentText，并触发signals
        return;
    contentText = text;
    emit textChanged(contentText);
}
