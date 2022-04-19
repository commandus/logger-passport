#ifndef LOGGER_PASSPORT_COLLECTION_H
#define LOGGER_PASSPORT_COLLECTION_H     1

#include <vector>
#include <map>
#include <string>
#include <mutex>

/**
 * @brief Each sensor have 8 bytes long MAC address
 */
typedef union {
    uint64_t u;
    uint8_t b[8];
} LOGGER_MAC;

/**
 * @brief Text serialization formats
 */
typedef enum {
    LOGGER_OUTPUT_FORMAT_PG = 0,
    LOGGER_OUTPUT_FORMAT_MYSQL = 1,
    LOGGER_OUTPUT_FORMAT_FB = 2,
    LOGGER_OUTPUT_FORMAT_SQLITE = 3,

    LOGGER_OUTPUT_FORMAT_JSON = 4,		    	///< default
    LOGGER_OUTPUT_FORMAT_TEXT = 5,	    	    ///< text
    LOGGER_OUTPUT_FORMAT_TABLE = 6		        ///< table
} LOGGER_OUTPUT_FORMAT;

/**
 * @brief Each sensor have MAC address
 */
class SensorMAC {
public:
    LOGGER_MAC mac;
    SensorMAC();
    SensorMAC(const SensorMAC&value);
    SensorMAC(const std::string &value);
    SensorMAC &operator=(uint64_t value);
    SensorMAC &operator=(const SensorMAC &value);
    SensorMAC &operator=(const std::string &value);

    bool operator==(const SensorMAC &another) const;
    bool operator!=(const SensorMAC &another) const;
    bool operator==(uint64_t another) const;
    bool operator!=(uint64_t another) const;

    void set(const std::string &value);

    std::string toString() const;
};

/**
 * @brief sensor MAC address and coefficients
 * Each sensor has MAC address and 2, 3 or 9 coefficients to spline temperature using polynomial:
 * Usually sensor has 2 or 3 coefficients to spline temperature using polynomial:
 * Tpoly = a * Traw + b + c * Traw * Traw
 * By default, coefficient C = 0.0 (when two coefficients are used).
 * Some sensors have 9 coefficients to spline temperature using polynomial in 4 temperature ranges:
 * -273 .. -3 a, b, c
 * [-3 .. 3] a1, b1, c1
 * 3 .. 30] as is
 * 30 .. max a2, b2, c2
 * Tpoly = a * Traw + b + c * Traw * Traw in range <-3C
 * Tpoly = a1 * Traw + b1 + c1 * Traw * Traw in range >=-3C and <= 3
 * Tpoly = Traw in range  >3 and <= 30
 * Tpoly = a2 * Traw + b2 + c2 * Traw * Traw in range >30
 */
class SensorCoefficients {
public:
    SensorMAC mac;
    std::vector <double> coefficients;
    std::string toJsonString() const;
    std::string toString() const;
    std::string toTableString() const;
    std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
    double calc(double temperature) const;
};

/**
 * @brief Sensors plume have one or more sensors
 * Plume identifier consists of year and plume serial number in the year starting with 1.
 */
class LoggerPlumeId {
	public:
		int plume;
		int year;
		LoggerPlumeId();
		LoggerPlumeId(const LoggerPlumeId &value);
		LoggerPlumeId(int aPlume, int aYear);
		LoggerPlumeId& operator=(const LoggerPlumeId& other);
		bool operator<(const LoggerPlumeId &another) const;
		bool operator==(const LoggerPlumeId &another) const;
		bool operator!=(const LoggerPlumeId &another) const;

		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};

/**
 * @brief Each sensors plume have logger passport with coefficients for each sensor to spline raw temperature data.
 */
class LoggerPassport {
	public:
        /**
         * Scanning multiple files can cause old files collision. To resolve it, each record have
         * last modification time.
         * Not used.
         */
		time_t modificationTime;
        /**
         * Year and serial number
         */
		LoggerPlumeId id;
        /**
         * Sensors array
         */
        std::vector<SensorCoefficients> sensors;
        /**
         * Optional file name where passport saved
         */
        std::string name;

		LoggerPassport();
		LoggerPassport(const LoggerPassport& value);
		LoggerPassport(time_t aModificationTime);
		virtual ~LoggerPassport();
		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;
};


/**
 * Helper parser tokens
 */
typedef enum {
	TOK_YEAR_PLUME = 1,
	TOK_MAC = 3,
	TOK_COEFF = 4,
	TOK_END = 5
} PASSPORT_TOKEN;

/**
 * LoggerPassportCollection is collection of plumes.
 * Plumes are loaded from files or streams.
 * It parses passport files in two formats: text files or JSON array.
 * @brief abstract class have virtual methods relates to the storage where collection resides.
 */
class LoggerPassportCollection {
	private:
        /**
         * File parser routine return next token
         * @param strm input stream
         * @return token string
         */
		std::string next(std::istream& strm);
	public:
		// Abstract methods
        /**
         * Return count of passports
         * @return count of passports
         */
		virtual size_t count() const = 0;
        /**
         * Return plume identifiers in the storage
         * @param retval returned plume identifiers
         * @param offset e.g. 0
         * @param limit e.g. 100
         * @return count of added identifiers to the retval parameter
         */
		virtual size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const = 0;
        /**
         * Return passport from specified identifier
         * @param id plume identifier
         * @return pointer to the passport if exists, NULL if not found
         */
		virtual const LoggerPassport *get(const LoggerPlumeId &id) const = 0;
        /**
         * Return pointer to the sensor
         * @param serialNo plume serial number
         * @param year yeat of production
         * @param mac sensor MAC address
         * @return NULL if not found
         */
        const SensorCoefficients *get(int serialNo, int year, uint64_t mac) const;
        /**
         * Add passport to the storage
         * @param value passport
         */
		virtual void push(LoggerPassport &value) = 0;
        /**
         * Remove passport form the storage
         * @param id plume identifier
         */
        virtual void remove(const LoggerPlumeId &id) = 0;
        /**
         * Clear storage
         */
        virtual void clear() = 0;
        /**
         * Parse JSON file stream
         * @param modificationTime file last modification time, unix epoch in seconds
         * @param name optional file name for reference
         * @param strm input stream
         * @return parsed plume passports count
         */
        int parseJson(
                time_t modificationTime,
                const std::string &name, ///< optional resource name
                std::istream &strm
        );
        /**
         * Parse Text file stream
         * @param modificationTime file last modification time, unix epoch in seconds
         * @param name optional file name for reference
         * @param strm input stream
         * @return parsed plume passports count
         */
		int parseText(time_t modificationTime, const std::string &name, std::istream &strm);
        /**
         * Parse passports from the string
         * @param modificationTime time of creation
         * @param value Passords in the string
         * @return parsed plume passports count
         */
		int loadString(time_t modificationTime, const std::string &value, const bool isJson = false);
        /**
         * Parse passports from the file or directory
         * @param fileName Input file name
         * @param fileSuffix e.g. ".txt"
         * @return parsed plume passports count
         */
		int loadFile(const std::string &fileName, const std::string &fileSuffix);
        /**
         * Parse passports from the files or directories enlisted in the fileNames
         * @param fileNames Input file or directory name array
         * @param fileSuffix e.g. ".txt"
         * @return parsed plume passports count
         */
		int loadFiles(const std::vector<std::string> &fileNames, const std::string &fileSuffix);
        /**
         * Serialize passports as JSON string
         * @return JSON string
         */
		std::string toJsonString() const;
        std::string toString() const;
        std::string toTableString() const;
        std::string sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const;

        // Helper overload routines
        /**
         * Get passport by integers
         * @param serialNo serial number starting with 1
         * @param year year 00..99 minus 2000
         * @return pointer to the passport ot NULL
         */
        const LoggerPassport *get(int serialNo, int year) const;
        /**
         * Remove passport form the storage
         * @param id plume identifier
         */
        void remove(int serialNo, int year);

        /**
         * Calculate temperature
         * @param serialNo plume serial number starting with 1
         * @param year plume year - 2000
         * @param sensor MAC address
         * @param temperature temperature
         * @return temperature
         */
        double calc(int serialNo, int year, uint64_t sensor, double temperature) const;
};

/**
 * @brief in-memory implementation of the LoggerPassportCollection abstract class
 */
class LoggerPassportMemory: public LoggerPassportCollection {
    private:
        mutable std::mutex mapMutex;
	public:
		std::map <LoggerPlumeId, LoggerPassport> values;
		LoggerPassportMemory();
		LoggerPassportMemory(const LoggerPassportMemory& value);
		virtual ~LoggerPassportMemory();
		size_t count() const override;
		size_t ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const override;
		const LoggerPassport *get(const LoggerPlumeId &id) const override;
		void push(LoggerPassport &value) override;
        void remove(const LoggerPlumeId &id) override;
        void clear() override;
};

#endif
