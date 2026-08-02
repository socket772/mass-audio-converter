#include "utils.h"
#include <filesystem>
#include <string>

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

bool isSupportedFormat(std::string fileExtension)
{
    if (formatsList().contains(fileExtension))
        return true;
    else
        return false;
}

std::vector<std::string> filteredFilesFolder(std::string folderPath)
{
    std::vector<std::string> filteredFiles;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_directory() && entry.exists() && isSupportedFormat(entry.path().extension()))
            filteredFiles.push_back(entry.path().string());
    }

    return filteredFiles;
}