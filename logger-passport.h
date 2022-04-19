/*
 * @brief Load passports from the directory and start listen for changes
 * @file logger-passport.h
 */
#ifndef LOGGER_PASSPORT_H_
#define LOGGER_PASSPORT_H_	1

#include <string>
#include <functional>

typedef std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message)>
LOG_CALLBACK;

/**
 * Load passports from the directory. Listen for changes in the directory
 * @param config return pointer to LoggerPassportCollection if success
 * @return descriptor
 */
void *startPassportDirectory(
    std::string passportDir, ///< passport files root
    LOG_CALLBACK onLog      ///< log callback
);

/**
 * Free resources
 * @param config set pointer to LoggerPassportCollection to null
 * @return true success
 */
void stopPassportDirectory(
	void *descriptor
);

/**
 * Calc temperature
 * @param descriptor passport collection descriptor
 * @param serialNo plume serial number
 * @param year plume production year
 * @param sensor sensor MAC address
 * @param value raw temperature
 * @return temperature
 */
double calcTemparature(
    void *descriptor,
    int serialNo,
    int year,
    uint64_t sensor,
    double value
);

#endif
