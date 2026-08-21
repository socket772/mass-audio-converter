#include "converterworker.h"
#include "utils.h"
#include <qdebug.h>

void ConverterWorker::run()
{
    while (true) {
        // Blocco dove accedo al file tramite mutex, cosi viene distrutto subito

        qInfo() << "w" + std::to_string(threadId) + ":" << "aspetto lock";
        sharedMutex.lock();
        qInfo() << "w" + std::to_string(threadId) + ":" << "lock preso";
        if (filesList.empty()) {
            qInfo() << "w" + std::to_string(threadId) + ":" << "lock lasciato";
            sharedMutex.unlock();
            qInfo() << "w" + std::to_string(threadId) + ":" << "file finiti";
            break;
        }

        // estrai un file dalla pila (LIFO)
        filePath = filesList.back();
        filesList.pop_back();
        sharedMutex.unlock();
        qInfo() << "w" + std::to_string(threadId) + ":" << "lock lasciato";

        qInfo() << "w" + std::to_string(threadId) + ":" << "emit progress";
        emit progress();
    }
    qInfo() << "w" + std::to_string(threadId) + ":" << "Finished";
    // thread()->quit();
}
