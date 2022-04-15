#ifndef LOGGER_PASSPORT_COLLECTION_H
#define LOGGER_PASSPORT_COLLECTION_H     1

#include <vector>
#include <map>
#include <string>

typedef enum {
    LOGGER_OUTPUT_FORMAT_PG = 0,
    LOGGER_OUTPUT_FORMAT_MYSQL = 1,
    LOGGER_OUTPUT_FORMAT_FB = 2,
    LOGGER_OUTPUT_FORMAT_SQLITE = 3,

  	LOGGER_OUTPUT_FORMAT_JSON = 4,		    	///< default
	LOGGER_OUTPUT_FORMAT_TEXT = 5,	    	    ///< text
    LOGGER_OUTPUT_FORMAT_TABLE = 6		        ///< table

} LOGGER_OUTPUT_FORMAT;

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
		LoggerPassportItem();
		LoggerPassportItem(const LoggerPassportItem& value);
		LoggerPassportItem(time_t modificationTime);
		virtual ~LoggerPassportItem();
		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

class LoggerPassportCollection {
	private:
		size_t listDir(std::vector<std::string> &retval, const std::string &path, const std::string &fileSuffix);
	public:
		virtual size_t count() const = 0;
		virtual size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const = 0;
		virtual const LoggerPassportItem *get(const LoggerPlumeId &id) const = 0;
		virtual void push(LoggerPassportItem &value) = 0;

		int loadStream(time_t modificationTime, const std::istream &strm);
		int loadString(time_t modificationTime, const std::string &value);
		int loadFile(const std::string &fileName, const std::string &fileSuffix);
		int loadFiles(const std::vector<std::string> &fileNames, const std::string &fileSuffix);

		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

class LoggerPassportMemory: public LoggerPassportCollection {
	public:
		std::map <LoggerPlumeId, LoggerPassportItem> values;
		LoggerPassportMemory();
		LoggerPassportMemory(const LoggerPassportMemory& value);
		virtual ~LoggerPassportMemory();
		virtual size_t count() const override;
		virtual size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const  override;
		virtual const LoggerPassportItem *get(const LoggerPlumeId &id) const override;
		virtual void push(LoggerPassportItem &value) override;
};

#endif
