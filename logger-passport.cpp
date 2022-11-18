#include "logger-passport.h"
#include "logger-plume-collection.h"
#if __cplusplus >= 201103L
// #define ENABLE_WATCHER 1
#else
#undef ENABLE_WATCHER
#endif

#ifdef ENABLE_WATCHER
#include "filewatch.hpp"
#endif

#include "utilfile.h"

#define  MODULE_CODE    11

class PassportServiceConfig {
public:
    // service descriptor
    LoggerPlumeCollection *passports;
#ifdef ENABLE_WATCHER
    // watcher
    filewatch::FileWatch<std::string> *watcher;
#endif
    // log callback
    LOG_CALLBACK onLog;
    // passport files root
    std::vector<std::string> passportDirs;

    PassportServiceConfig();
    virtual ~PassportServiceConfig();
#ifdef ENABLE_WATCHER
    void reload(const std::string &fileName, const filewatch::Event changeType);
#endif
    void load();
};

PassportServiceConfig::PassportServiceConfig()
    : passports(nullptr),
#ifdef ENABLE_WATCHER
    watcher(nullptr),
#endif
    onLog(nullptr)
{

}

PassportServiceConfig::~PassportServiceConfig()
{
}

#ifdef ENABLE_WATCHER
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
#endif

void PassportServiceConfig::load() {
    this->passports->startModification();
    this->passports->clear();
    this->passports->loadFiles(this->passportDirs, "");
    this->passports->finishModification();
}

/**
 * Load passports from the directory. Listen for changes in the directory
 */
void *startPassportDirectory(
    const std::string &passportDir, ///< passport files root
    LOG_CALLBACK onLog,              ///< log callback
    bool enableMonitor
)
{
    PassportServiceConfig *config = new PassportServiceConfig();
    bool exists = (!passportDir.empty()) && util::fileExists(passportDir);
    if (exists) {
        config->passportDirs.push_back(passportDir);
        config->passports = new LoggerPlumeMemory();
        config->onLog = onLog;
#ifdef ENABLE_WATCHER
        if (enableMonitor) {
            config->watcher = new filewatch::FileWatch<std::string>(config->passportDirs[0],
                [config](const std::string &path,
                         const filewatch::Event changeType) {
                    config->reload(path, changeType);
                }
            );
        }
#endif
        config->load();
    }
    return config;
}

/**
 * Load passports from the directories or files. Listen for changes in the directory
 */
void *startPassportDirectory(
    const std::vector<std::string> &passportDirs,   ///< passport files or directories list
    LOG_CALLBACK onLog,                              ///< log callback
    bool enableMonitor
)
{
    PassportServiceConfig *config = new PassportServiceConfig();
    bool exists = (!passportDirs.empty()) && (!passportDirs[0].empty()) && util::fileExists(passportDirs[0]);
    if (exists) {
        config->passports = new LoggerPlumeMemory();
        config->passportDirs = passportDirs;
        config->onLog = onLog;
#ifdef ENABLE_WATCHER
        if (enableMonitor) {
            config->watcher = new filewatch::FileWatch<std::string>(config->passportDirs[0],
                [config](const std::string &path,
                         const filewatch::Event changeType) {
                    config->reload(path, changeType);
                }
            );
        }
#endif
        config->load();
    }
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
    if (!config)
        return;
#ifdef ENABLE_WATCHER
    if (config->watcher) {
        delete config->watcher;
        config->watcher = nullptr;
    }
#endif
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
    if (!config->passports)
        return false;
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
    std::vector<LoggerPlumeId> identifiers;
    size_t r = config->passports->ids(identifiers, year, plume, offset, count);
    if (!retVal | retType == FORMAT_PASSPORT_NONE)
        return r;
    for (std::vector<LoggerPlumeId>::const_iterator it(identifiers.begin()); it != identifiers.end(); it++) {
        const LoggerPlume *p = config->passports->get(*it);
        if (!p)
            continue;
        switch (retType) {
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
    if (!config->passports)
        return value;
    return config->passports->calc(serialNo, year, sensor, value);
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
    if (!config->passports)
        return value;
    return config->passports->calc(mac, value);
}
