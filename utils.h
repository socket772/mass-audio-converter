#ifndef UTILS_H
#define UTILS_H
#include <QStringList>
#include <mutex>
#include <qcontainerfwd.h>
#include <qwindowdefs.h>
#include <string>

struct formDataStruct
{
    std::string inputFolder;
    std::string outputFolder;
    std::string format;
    int threadsNumber;
    bool overwrite;
};

inline constexpr std::size_t formatsCount = 10;
const QStringList &formatsList();
extern formDataStruct formData;
extern QWidgetList widgetList;
extern std::vector<std::string> filesList;
extern std::mutex sharedMutex;
std::string prettyFormData(formDataStruct formData);
std::vector<std::string> filteredFilesFolder(std::string folderPath);
bool isSupportedFormat(std::string fileExtension);

#endif // UTILS_H