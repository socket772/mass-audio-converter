#ifndef UTILS_H
#define UTILS_H
#include <QStringList>

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
std::string prettyFormData(formDataStruct formData);

#endif // UTILS_H