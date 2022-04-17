#ifndef LOGGER_PASSPORT_COLLECTION_H
#define LOGGER_PASSPORT_COLLECTION_H     1

#include <vector>
#include <map>
#include <string>

#include "logger-passport.h"

typedef union {
    uint64_t u;
    uint8_t b[8];
} LOGGER_MAC;

typedef enum {
    LOGGER_OUTPUT_FORMAT_PG = 0,
    LOGGER_OUTPUT_FORMAT_MYSQL = 1,
    LOGGER_OUTPUT_FORMAT_FB = 2,
    LOGGER_OUTPUT_FORMAT_SQLITE = 3,

    LOGGER_OUTPUT_FORMAT_JSON = 4,		    	///< default
    LOGGER_OUTPUT_FORMAT_TEXT = 5,	    	    ///< text
    LOGGER_OUTPUT_FORMAT_TABLE = 6		        ///< table

} LOGGER_OUTPUT_FORMAT;

class SensorMAC {
public:
    LOGGER_MAC mac;
    SensorMAC();
    SensorMAC(const SensorMAC&value);
    SensorMAC(const std::string &value);
    SensorMAC &operator=(uint64_t value);
    SensorMAC &operator=(const SensorMAC &value);
    SensorMAC &operator=(const std::string &value);
    void set(const std::string &value);

    std::string toJsonString() const;
    std::string toString() const;
    std::string toTableString() const;
    std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

class SensorCoefficients {
public:
    SensorMAC mac;
    std::vector <double> coefficents;

    std::string toJsonString() const;
    std::string toString() const;
    std::string toTableString() const;
    std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

class LoggerPlumeId {
	public:
		int plume;
		int year;
		LoggerPlumeId();
		LoggerPlumeId(const LoggerPlumeId &value);
		LoggerPlumeId(int plume, int year);
		LoggerPlumeId& operator=(const LoggerPlumeId& other);
		bool operator<(const LoggerPlumeId &another) const;
		bool operator==(const LoggerPlumeId &another) const;
		bool operator!=(const LoggerPlumeId &another) const;

		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

class LoggerPassportItem {
	public:
		time_t modificationTime;
		LoggerPlumeId id;
        std::vector<SensorCoefficients> sensors;
        std::string name;

		LoggerPassportItem();
		LoggerPassportItem(const LoggerPassportItem& value);
		LoggerPassportItem(time_t modificationTime);
		virtual ~LoggerPassportItem();
		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;

};

// parser tokens
typedef enum {
	TOK_YEAR_PLUME = 1,
	TOK_MAC = 3,
	TOK_COEFF = 4,
	TOK_END = 5
} PASSPORT_TOKEN;

/**
 * @brief abstract class
 */

class LoggerPassportCollection {
	private:
		size_t listDir(std::vector<std::string> &retval, const std::string &path, const std::string &fileSuffix);
		std::string next(std::istream& strm);
	public:
		// Abstract methods
		virtual size_t count() const = 0;
		virtual size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const = 0;
		virtual const LoggerPassportItem *get(const LoggerPlumeId &id) const = 0;
		virtual void push(LoggerPassportItem &value) = 0;

		int parse(time_t modificationTime, const std::string &name, std::istream &strm);
		int loadString(time_t modificationTime, const std::string &value);
		int loadFile(const std::string &fileName, const std::string &fileSuffix);
		int loadFiles(const std::vector<std::string> &fileNames, const std::string &fileSuffix);

		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

/**
 * @brief in-memory 
 */
class LoggerPassportMemory: public LoggerPassportCollection {
	public:
		std::map <LoggerPlumeId, LoggerPassportItem> values;
		LoggerPassportMemory();
		LoggerPassportMemory(const LoggerPassportMemory& value);
		virtual ~LoggerPassportMemory();
		size_t count() const override;
		size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const  override;
		const LoggerPassportItem *get(const LoggerPlumeId &id) const override;
		void push(LoggerPassportItem &value) override;
};

#endif
