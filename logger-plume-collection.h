#ifndef LOGGER_PASSPORT_COLLECTION_H
#define LOGGER_PASSPORT_COLLECTION_H     1

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <inttypes.h>

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
    std::vector<std::vector <double> > coefficients;  ///<  array of array: 1 for any temperature (one range), 3 arrays for four ranges
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
class LoggerPlume {
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
         * Sensors coefficients
         */
        std::vector<SensorCoefficients> sensors;
        /**
         * Optional file name where passport saved
         */
        std::string name;

		LoggerPlume();
		LoggerPlume(const LoggerPlume& value);
		LoggerPlume(time_t aModificationTime);
		virtual ~LoggerPlume();
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
 * LoggerPlumeCollection is collection of plumes.
 * Plumes are loaded from files or streams.
 * It parses passport files in two formats: text files or JSON array.
 * @brief abstract class have virtual methods relates to the storage where collection resides.
 */
class LoggerPlumeCollection {
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
         * @param year 0- any, >0- filter by year
         * @param plume 0- any, >0- filter by plume
         * @param offset e.g. 0
         * @param limit e.g. 100
         * @return count of added identifiers to the retval parameter
         */
		virtual size_t ids(std::vector<LoggerPlumeId> &retval,
            int year, int plume, size_t offset, size_t limit) const = 0;
        /**
         * Return passport from specified identifier
         * @param id plume identifier
         * @return pointer to the passport if exists, NULL if not found
         */
		virtual const LoggerPlume *get(const LoggerPlumeId &id) const = 0;
        /**
         * Return pointer to the sensor
         * @param mac sensor MAC address
         * @return NULL if not found
         */
        virtual const SensorCoefficients *getSensor(uint64_t mac) const = 0;
        /**
         * Return pointer to the sensor
         * @param serialNo plume serial number
         * @param year yeat of production
         * @param no sensor number 0..
         * @return NULL if not found
         */
        const SensorCoefficients *getSensor(int serialNo, int year, int no) const;
        /**
         * Add passport to the storage
         * @param value passport
         */
		virtual void push(LoggerPlume &value) = 0;
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
        const LoggerPlume *get(int serialNo, int year) const;
        /**
         * Remove passport form the storage
         * @param id plume identifier
         */
        void remove(int serialNo, int year);

        /**
         * Calculate temperature
         * @param serialNo plume serial number starting with 1
         * @param year plume year - 2000
         * @param no sensor no 0..
         * @param temperature temperature
         * @return temperature
         */

        double calc(int serialNo, int year, int no, double temperature) const;
        /**
         * Calculate temperature
         * @param mac MAC address
         * @param temperature temperature
         * @return temperature
         */
        double calc(uint64_t mac, double temperature) const;

        virtual void startModification() = 0;
        virtual void finishModification() = 0;

        virtual ~LoggerPlumeCollection();
};

/**
 * @brief in-memory implementation of the LoggerPlumeCollection abstract class
 */
class LoggerPlumeMemory: public LoggerPlumeCollection {
    private:
        mutable std::mutex mapMutex;
        std::map <uint64_t, const SensorCoefficients*> macIndex;
        void buildMacIndex();
	public:
		std::map <LoggerPlumeId, LoggerPlume> values;
		LoggerPlumeMemory();
		LoggerPlumeMemory(const LoggerPlumeMemory& value);
		virtual ~LoggerPlumeMemory();
		size_t count() const override;
		size_t ids(std::vector<LoggerPlumeId> &retval,
            int year, int plume, size_t offset, size_t limit) const override;
		const LoggerPlume *get(const LoggerPlumeId &id) const override;
        const SensorCoefficients *getSensor(uint64_t mac) const override;
		void push(LoggerPlume &value) override;
        void remove(const LoggerPlumeId &id) override;
        void clear() override;

        void startModification() override;
        void finishModification() override;

};

#endif
