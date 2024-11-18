#include "enum_desc.h"

#include <QCoreApplication>

QString EnumDesc::name(int value) const
{
    QByteArray str = untranslatedName(value);
    if(str.isEmpty())
        return QString();

    return QCoreApplication::translate(
                translationContext.constData(),
                str.constData());
}
