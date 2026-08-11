#include "converterworker.h"
#include "utils.h"
#include <algorithm>
#include <string>

void ConverterWorker::run()
{
    while (true) {
        std::string filePath;
        // Blocco dove accedo al file tramite mutex, cosi viene distrutto subito
        {
            sharedMutex.lock();
            if (filesList.empty()) {
                break;
            }

            // estrai un file dalla pila (LIFO)
            filePath = filesList.back();
            filesList.pop_back();
        }
    }
}
