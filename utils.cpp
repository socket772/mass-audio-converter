#include "utils.h"

const QStringList &formatsList()
{
    static const QStringList list{QStringLiteral("mp3"),
                                  QStringLiteral("m4a"),
                                  QStringLiteral("flac"),
                                  QStringLiteral("ogg"),
                                  QStringLiteral("wav"),
                                  QStringLiteral("aac"),
                                  QStringLiteral("m4b"),
                                  QStringLiteral("oga"),
                                  QStringLiteral("opus"),
                                  QStringLiteral("webm")};
    return list;
};