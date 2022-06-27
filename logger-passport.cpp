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
    std::vector<std::string> passportDirs;

    PassportServiceConfig();
    virtual ~PassportServiceConfig();

    void reload(const std::string &fileName, const filewatch::Event changeType);

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
        }
        onLog(this, 3, MODULE_CODE, 0, "File " + fileName + " " + t);
    }
}

void PassportServiceConfig::load() {
    this->passports->startModification();
    this->passports->clear();
    this->passports->loadFiles(this->passportDirs, "");
    this->passports->finishModification();
}

/**
 * Load passports from the directory. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return descriptor
 */
void *startPassportDirectory(
    const std::string &passportDir, ///< passport files root
    LOG_CALLBACK onLog              ///< log callback
)
{
    PassportServiceConfig *config = new PassportServiceConfig();
    config->passportDirs.push_back(passportDir);
    config->passports = new LoggerPlumeMemory();
    config->onLog = onLog;

    if (!passportDir.empty()) {
        config->watcher = new filewatch::FileWatch<std::string>(config->passportDirs[0],
        [config](const std::string &path, const filewatch::Event changeType) {
                config->reload(path, changeType);
            }
        );
    }
    config->load();
    return config;
}

/**
 * Load passports from the directories or files. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return descriptor
 */
void *startPassportDirectory(
    const std::vector<std::string> &passportDirs,   ///< passport files or directories list
    LOG_CALLBACK onLog                              ///< log callback
)
{
    PassportServiceConfig *config = new PassportServiceConfig();
    config->passports = new LoggerPlumeMemory();
    config->passportDirs = passportDirs;
    config->onLog = onLog;

    if (!passportDirs.empty()) {
        config->watcher = new filewatch::FileWatch<std::string>(config->passportDirs[0],
        [config](const std::string &path, const filewatch::Event changeType) {
                config->reload(path, changeType);
            }
        );
    }
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

/**
 * Return true if plume has passport.
 * Passport descriptor must be initialized.
 * @param descriptor passport collection descriptor
 * @param retType request return passport
 * @param retVal if not NULL, return sensor passports as JSON or text string
 * @param serialNo plume serial number
 * @param year plume production year
 * @return true- has passport
 */
bool hasPassport(
    void *descriptor,
    FORMAT_PASSPORT_TYPE retType,
    std::string *retVal,
    int serialNo,
    int year
)
{
    if (!descriptor)
        return false;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    const LoggerPlume *p = config->passports->get(serialNo, year);
    if (p && retVal) {
        switch(retType) {
            case FORMAT_PASSPORT_TEXT:
                *retVal = p->toString();
                break;
            case FORMAT_PASSPORT_JSON:
                *retVal = p->toJsonString();
                break;
            case FORMAT_PASSPORT_TABLE:
                *retVal = p->toTableString();
                break;
            default:
                break;
        }
    }
    return p!= nullptr;
}

size_t countPassports(
    void *descriptor,
    FORMAT_PASSPORT_TYPE retType,
    std::vector<std::string> *retVal,
    int year,
    int plume,
    size_t offset,
    size_t count
)
{
    if (!descriptor)
        return 0;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    if (!config->passports)
        return 0;
    size_t r = config->passports->count();
    if (!retVal)
        return r;
    std::vector<LoggerPlumeId> identifiers;
    config->passports->ids(identifiers, year, plume, offset, count);
    for (std::vector<LoggerPlumeId>::const_iterator it(identifiers.begin()); it != identifiers.end(); it++) {
        const LoggerPlume *p = config->passports->get(*it);
        if (!p)
            continue;
        switch(retType) {
            case FORMAT_PASSPORT_TEXT:
                retVal->push_back(p->toString());
                break;
            case FORMAT_PASSPORT_JSON:
                retVal->push_back(p->toJsonString());
                break;
            case FORMAT_PASSPORT_TABLE:
                retVal->push_back(p->toTableString());
                break;
        }
    }
    return r;
}

/**
 * Check has sensor passport. If second parameter is not NULL, return coefficients
 * @param descriptor passport descriptor
 * @param retCoefficients If not NULL, return pointer to array of array: 1 for any temperature (one range), 3 arrays for four ranges
 * @param serialNo plume serial number
 * @param year plume production year - 2000
 * @param seqno sensor number 1..
 * @return true- plume has sensor and plume has passport
 */
bool hasSensor(
    void *descriptor,
    const std::vector<std::vector <double> > **retCoefficients,
    int serialNo,
    int year,
    int seqno
)
{
    if (!descriptor)
        return false;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    const SensorCoefficients *c = config->passports->getSensor(serialNo, year, seqno);
    if (retCoefficients)
        *retCoefficients = &c->coefficients;
    return c != nullptr;
}

/**
 * Check has sensor passport. If second parametr is not NULL, return coefficients
 * @param descriptor passport descriptor
 * @param retCoefficients If not NULL, return pointer to array of array: 1 for any temperature (one range), 3 arrays for four ranges
 * @param mac MAC address
 * @return true- plume has sensor and plume has passport
 */
bool hasSensor(
    void *descriptor,
    const std::vector<std::vector <double> > **retCoefficients,
    uint64_t mac
)
{
    if (!descriptor)
        return false;
    PassportServiceConfig *config = (PassportServiceConfig *) descriptor;
    const SensorCoefficients *c = config->passports->getSensor(mac);
    if (retCoefficients)
        *retCoefficients = &c->coefficients;
    return c != nullptr;
}

double calcTemperature(
    void *descriptor,
    int serialNo,
    int year,
    int sensor,
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
