#include "utils.h"
#include <filesystem>
#include <string>

/*
 * Lista di formati supportati per l'output
 */
const QStringList &formatsList() {
  static const QStringList list{QStringLiteral("mp3"),  QStringLiteral("m4a"),
                                QStringLiteral("flac"), QStringLiteral("ogg"),
                                QStringLiteral("wav"),  QStringLiteral("aac"),
                                QStringLiteral("m4b"),  QStringLiteral("oga"),
                                QStringLiteral("opus"), QStringLiteral("webm")};
  return list;
};

formDataStruct formData{};
QWidgetList widgetList{};
std::vector<std::string> filesList{};

/*
 * Stampa di debug per formDataStruct
 */
std::string prettyFormData(formDataStruct formData)
{
    std::string temp = "false";
    if (formData.overwrite)
        temp = "true";

    return "format: " + formData.format + " | inputFolder: " + formData.inputFolder
           + " | outputFolder: " + formData.outputFolder + " | overwrite: " + temp
           + " | threadsNumber: " + std::to_string(formData.threadsNumber);
}

/*
 * Ritorna vero se il formato selezionato è nella lista formatsList
 */
bool isSupportedFormat(std::string fileExtension)
{
    if (formatsList().contains(fileExtension))
        return true;
    else
        return false;
}

/*
 * Questa funzione ottiene una lista filtrata di tutti i file in una cartella, non in modo ricorsivo
 * il filtro è preso da formatsList
*/
std::vector<std::string> filteredFilesFolder(std::string folderPath)
{
    std::vector<std::string> filteredFiles;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_directory() && entry.exists() && isSupportedFormat(entry.path().extension()))
            filteredFiles.push_back(entry.path().string());
    }

    return filteredFiles;
}