#include "utils.h"

const QStringList &formatsList() {
  static const QStringList list{QStringLiteral("mp3"),  QStringLiteral("m4a"),
                                QStringLiteral("flac"), QStringLiteral("ogg"),
                                QStringLiteral("wav"),  QStringLiteral("aac"),
                                QStringLiteral("m4b"),  QStringLiteral("oga"),
                                QStringLiteral("opus"), QStringLiteral("webm")};
  return list;
};

formDataStruct formData{};

std::string prettyFormData(formDataStruct formData)
{
    std::string temp = "false";
    if (formData.overwrite)
        temp = "true";

    return "format: " + formData.format + " | inputFolder: " + formData.inputFolder
           + " | outputFolder: " + formData.outputFolder + " | overwrite: " + temp
           + " | threadsNumber: " + std::to_string(formData.threadsNumber);
}