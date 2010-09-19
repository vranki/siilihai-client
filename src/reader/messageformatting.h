#ifndef MESSAGEFORMATTING_H_
#define MESSAGEFORMATTING_H_

#include <QString>
#include <QRegExp>

class MessageFormatting {
public:
    MessageFormatting();
    virtual ~MessageFormatting();
    static QString stripHtml(QString &txt);
    static QString sanitize(QString &txt); // strip html and newlines
    static QString replaceCharacters(QString &txt); // Convert < -> &lt; etc.
};

#endif /* MESSAGEFORMATTING_H_ */
