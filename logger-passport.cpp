/**
 *
 */

#include "logger-passport.h"
#include "logger-plume-collection.h"
#include "filewatch.hpp"

#define  MODULE_CODE    11

class PassportServiceConfig {
public:
    // service descriptor
    LoggerPlumeCollection *passports;
    // watcher
    filewatch::FileWatch<std::string> *watcher;
    // log callback
    LOG_CALLBACK onLog;
    // passport files root
    std::string passportDir;

    PassportServiceConfig();
    virtual ~PassportServiceConfig();

    void reload(const std::string &fileName, const filewatch::Event event);

    void load();
};

PassportServiceConfig::PassportServiceConfig()
    : passports(nullptr), watcher(nullptr), onLog(nullptr)
{

}

PassportServiceConfig::~PassportServiceConfig()
{
}

void PassportServiceConfig::reload(
    const std::string &fileName,
    const filewatch::Event changeType
) {
    load();
    if (onLog) {
        std::string t;
        switch (changeType) {
            case filewatch::Event::added:
                t = "added";
                break;
            case filewatch::Event::removed:
                t = "removed";
                break;
            case filewatch::Event::modified:
                t = "modified";
                break;
            case filewatch::Event::renamed_old:
                t = "renamed (old name)";
                break;
            case filewatch::Event::renamed_new:
                t = "renamed (new name)";
                break;
        };
        onLog(this, 3, MODULE_CODE, 0, "File " + fileName + " " + t + " in passport directory " + passportDir);
    }
}

void PassportServiceConfig::load() {
    this->passports->startModification();
    this->passports->clear();
    this->passports->loadFile(this->passportDir, "");
    this->passports->finishModification();
}

/**
 * Load passports from the directory. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return descriptor
 */
void *startPassportDirectory(
    std::string passportDir, ///< passport files root
    LOG_CALLBACK onLog      ///< log callback
)
{
    PassportServiceConfig *config = new PassportServiceConfig();
    config->passports = new LoggerPlumeMemory();
    config->watcher = new filewatch::FileWatch<std::string>(config->passportDir,
       [config](const std::string &path, const filewatch::Event changeType) {
           config->reload(path, changeType);
       }
    );
    config->load();
    return config;
}

/**
 * Free resources
 * @param config set pointer to LoggerPlumeCollection to null
 * @return true success
 */
void stopPassportDirectory(
    void *descriptor
)
{
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    if (config->watcher) {
        delete config->watcher;
        config->watcher = nullptr;
    }
    if (config->passports) {
        delete config->passports;
        config->passports = nullptr;
    }
}

double calcTemperature(
    void *descriptor,
    int serialNo,
    int year,
    uint64_t sensor,
    double value
)
{
    if (!descriptor)
        return value;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    config->passports->calc(serialNo, year, sensor, value);
}

/**
 * Calc temperature
 * @param descriptor passport collection descriptor
 * @param mac sensor MAC address
 * @param value raw temperature
 * @return temperature
 */
double calcTemperature(
    void *descriptor,
    uint64_t mac,
    double value
)
{
    if (!descriptor)
        return value;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    config->passports->calc(mac, value);
}
