#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <rapidjson/document.h>

#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>

#include "logger-passport-collection.h"
#include "list-file.h"
#include "errlist.h"

typedef enum {
    JSON_MODIFIED = 0,
    JSON_NAME = 1,
    JSON_ID = 2,
    JSON_ID_PLUME = 3,
    JSON_ID_YEAR = 4,
    JSON_SENSORS = 5,
    JSON_SENSORS_MAC = 6,
    JSON_SENSORS_COEFFICIENTS = 7
} JSON_PASSPORT_NAME;

const char *COMMA = ", ";

const char *JSON_PASSPORT_NAMES[] = {
    "modified",
    "name",
    "id",
    "plume",
    "year",
    "sensors",
    "mac",
    "coefficients"
};
// -------- SensorMAC --------

SensorMAC::SensorMAC()
{

}

SensorMAC::SensorMAC(const SensorMAC &value)
        : mac(value.mac)
{

}

SensorMAC::SensorMAC(const std::string &value)
{
    set(value);
}

std::string SensorMAC::toString() const
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << mac.u;
    return ss.str();
}

bool SensorMAC::operator==(const SensorMAC &another) const
{
    return mac.u == another.mac.u;
}

bool SensorMAC::operator!=(const SensorMAC &another) const
{
    return !(*this == another);
}

bool SensorMAC::operator==(uint64_t another) const
{
    return mac.u == another;
}

bool SensorMAC::operator!=(uint64_t another) const
{
    return !(*this == another);
}

void SensorMAC::set(const std::string &value)
{
    std::stringstream ss(value);
    ss >> std::hex >> mac.u;
}

SensorMAC &SensorMAC::operator=(uint64_t value)
{
    mac.u = value;
    return *this;
}

SensorMAC &SensorMAC::operator=(const SensorMAC &value)
{
    mac.u = value.mac.u;
    return *this;
}

SensorMAC &SensorMAC::operator=(const std::string &value)
{
    set(value);
    return *this;
}

// -------- LoggerPlumeId --------

std::string SensorCoefficients::toString() const
{
    std::stringstream ss;
    ss << mac.toString() << " ";
    for (auto it(coefficients.begin()); it != coefficients.end(); it++) {
        ss << std::fixed << std::setprecision(10) << *it << " ";
    }
    return ss.str();
}

std::string SensorCoefficients::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"" << JSON_PASSPORT_NAMES[JSON_SENSORS_MAC] << "\": \"" << mac.toString()
        << "\", \"" << JSON_PASSPORT_NAMES[JSON_SENSORS_COEFFICIENTS] << "\": [";
    bool isFirst = true;
    for (auto it(coefficients.begin()); it != coefficients.end(); it++) {
        if (isFirst)
            isFirst = false;
        else
            ss << COMMA;
        ss << std::fixed << std::setprecision(5) << *it;
    }
    ss << "]}";
    return ss.str();
}

std::string SensorCoefficients::toTableString() const
{
    return toString();
}

std::string SensorCoefficients::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
    return toString();
}

/**
 * @brief Return amended temperature
 * Tpoly = a * Traw + b + c * Traw * Traw in range <-3C
 * Tpoly = a1 * Traw + b1 + c1 * Traw * Traw in range >=-3C and <= 3
 * Tpoly = Traw in range  >3 and <= 30
 * Tpoly = a2 * Traw + b2 + c2 * Traw * Traw in range >30

 * @param temperature raw temperature
 * @return amended temperature
 */
double SensorCoefficients::calc(
    double temperature
)
{
    int sz = coefficients.size();
    int t = temperature;
    switch (sz) {
        case 2:
            return coefficients[0] * temperature + coefficients[1];
        case 3:
            return coefficients[0] * temperature + coefficients[1] + coefficients[2] * coefficients[2] * temperature;
        case 9:
            if (t < -3)
                return coefficients[0] * temperature + coefficients[1] + coefficients[2] * coefficients[2] * temperature;
            if (t <= 3)
                return coefficients[3] * temperature + coefficients[4] + coefficients[5] * coefficients[5] * temperature;
            if (t <= 30)
                return coefficients[6] * temperature + coefficients[7] + coefficients[8] * coefficients[8] * temperature;
            return temperature;
        default:
            return temperature;
    }
}

// -------- LoggerPlumeId --------

LoggerPlumeId::LoggerPlumeId()
	: plume(0), year(0)
{

}

LoggerPlumeId::LoggerPlumeId(
	const LoggerPlumeId &value
)
	: plume(value.plume), year(value.year)
{
	
}
LoggerPlumeId::LoggerPlumeId(
	int aPlume,
	int aYear
)
	: plume(aPlume), year(aYear)
{
	
}

LoggerPlumeId& LoggerPlumeId::operator=(
	const LoggerPlumeId& other
)
{
	plume = other.plume;
	year = other.year;
	return *this;
}

bool LoggerPlumeId::operator<(const LoggerPlumeId &another) const
{
	if (plume < another.plume)
		return true;
	if (plume > another.plume)
		return false;
	return year < another.year;
}

bool LoggerPlumeId::operator==(
	const LoggerPlumeId &another
) const
{
	return (plume == another.plume) && (year == another.year);
}

bool LoggerPlumeId::operator!=(
	const LoggerPlumeId &another
) const
{
	return !(*this == another);
}

std::string LoggerPlumeId::toJsonString() const
{
	std::stringstream ss;
	ss << "{" 
		<< "\"" << JSON_PASSPORT_NAMES[JSON_ID_PLUME] << "\": " << plume
		<< ", \"" << JSON_PASSPORT_NAMES[JSON_ID_YEAR] << "\": " << year
		<< "}";
	return ss.str();
}

std::string LoggerPlumeId::toString() const
{
	std::stringstream ss;
	ss << "N" << year << "-" << plume;
	return ss.str();
}

std::string LoggerPlumeId::toTableString() const
{
	return toString();
}

std::string LoggerPlumeId::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toString();
}

// -------- LoggerPassport --------

const static std::string noname;

LoggerPassport::LoggerPassport()
	: modificationTime(0), name(noname)
{
}

LoggerPassport::LoggerPassport(
	const LoggerPassport& value
)
	: modificationTime(value.modificationTime), id(value.id), name(value.name)
{

}

LoggerPassport::LoggerPassport(
	time_t aModificationTime
)
	: modificationTime(aModificationTime), name(noname)
{

}

LoggerPassport::~LoggerPassport()
{

}

std::string LoggerPassport::toJsonString() const
{
	std::stringstream ss;
	ss << "{"
		<< "\"" << JSON_PASSPORT_NAMES[JSON_MODIFIED] << "\": " << modificationTime
		<< ", \"" << JSON_PASSPORT_NAMES[JSON_NAME] << "\": \"" << name
		<< "\", \"" << JSON_PASSPORT_NAMES[JSON_ID] << "\": " << id.toJsonString()
		<< ", \""<< JSON_PASSPORT_NAMES[JSON_SENSORS] << "\": [";
	bool isFirst = true;
	for (auto it(sensors.begin()); it != sensors.end(); it++) {
		if (isFirst)
			isFirst = false;
		else
			ss << COMMA;
		ss << it->toJsonString();
	}
	ss << "]}";
	return ss.str();
}

std::string LoggerPassport::toString() const
{
	std::stringstream ss;
	ss << id.toString() << std::endl;
    for (std::vector<SensorCoefficients>::const_iterator it(sensors.begin()); it != sensors.end(); it++) {
        ss << it->mac.toString();
        int c = 0;
        for (std::vector<double>::const_iterator itc(it->coefficients.begin()); itc != it->coefficients.end(); itc++) {
            if (c && (c % 3 == 0))
                ss << std::endl << "\t\t";
            ss << "\t"  << std::fixed << std::setprecision(5) << *itc;
            c++;
        }
        ss << std::endl;
    }
	return ss.str();
}

std::string LoggerPassport::toTableString() const
{
	return toString();
}

std::string LoggerPassport::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toString();
}

// -------- LoggerPassportCollection --------

std::string LoggerPassportCollection::toJsonString() const
{
	std::vector<LoggerPlumeId> plumeIds;
	size_t cnt = ids(plumeIds, 0, 0);
	std::stringstream ss;
	ss << "[";
	bool firstItem = true;
	for (std::vector<LoggerPlumeId>::const_iterator it(plumeIds.begin()); it != plumeIds.end(); it++) {
		const LoggerPassport *v = get(*it);
		if (!v)
			continue;
		if (firstItem)
			firstItem = false;
		else
			ss << COMMA;
		ss << v->toJsonString();
	}
	ss << "]";
	return ss.str();
}

std::string LoggerPassportCollection::toString() const
{
    std::vector<LoggerPlumeId> plumeIds;
    size_t cnt = count();
    ids(plumeIds, 0, cnt);
    std::stringstream ss;
    for (std::vector<LoggerPlumeId>::const_iterator it(plumeIds.begin()); it != plumeIds.end(); it++) {
        const LoggerPassport *p = get(*it);
        if (p)
            ss << p->toString();
    }
	return ss.str();
}

std::string LoggerPassportCollection::toTableString() const
{
	return toJsonString();
}

std::string LoggerPassportCollection::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toJsonString();
}

const LoggerPassport *LoggerPassportCollection::get(int serialNo, int year) const
{
    return get(LoggerPlumeId(serialNo, year));
}

/**
 * Remove passport form the storage
 * @param id plume identifier
 */
void LoggerPassportCollection::remove(
    int serialNo,
    int year
)
{
    remove(LoggerPlumeId(serialNo, year));
}

static void skipComments(
	std::istream& strm
)
{
	while (!strm.eof()) {
		char c;
		strm >> c;
		if (strm.bad())
			break;
		if (c == 10)
			break;
	}
}

std::string LoggerPassportCollection::next(
	std::istream& strm
)
{
	std::stringstream ss;
	char c;
	// skip spaces
	while (!strm.eof()) {
		if (!strm.get(c))
			break;
		if (c <= 32)
			continue;
		if (c == '#')
			skipComments(strm);
		else
			ss << c;
		break;
	}
	
	while (!strm.eof()) {
		if (!strm.get(c))
			break;
		if (c <= 32)
			break;
		if (c == '#')
			skipComments(strm);
		else
			ss << c;
	}
	return ss.str();
}

static bool putYearPlume(
        LoggerPassport &retVal,
        const std::string &value
)
{
	size_t p = value.find('-');
	if (p == std::string::npos)
		return false;
	std::string sYear = value.substr(1, p);
    retVal.id.year = strtol(sYear.c_str(), nullptr, 10);
	std::string sPlume= value.substr(p + 1);
    retVal.id.plume = strtol(sPlume.c_str(), nullptr, 10);
	return true;
}

static bool putMac(
	SensorCoefficients &retVal,
	const std::string& value
)
{
	uint64_t m = strtoull(value.c_str(), nullptr, 16);
	if (!m)
		return false;
    retVal.mac = m;
	
	return true;
}

static bool putCoefficient(
	SensorCoefficients& retval,
	std::string& value
)
{
	size_t p = value.find(',');
	if (p != std::string::npos)
		value[p] = '.';
	double c = strtod(value.c_str(), nullptr);
	retval.coefficients.push_back(c);
	return true;
}

static bool isHex(
	const std::string& value
)
{
	for (int i = 0; i < value.size(); i++) {
		if (isxdigit(value[i]))
			continue;
		return false;
	}
	return true;
}

int LoggerPassportCollection::parseJson(
        time_t modificationTime,
        const std::string &name, ///< optional resource name
        std::istream &strm
)
{
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
    rapidjson::IStreamWrapper rStream(strm);
    doc.ParseStream(rStream);
    if (!doc.IsArray())
        return ERR_LOGGER_PASSPORT_INVALID_JSON;

    for (int i = 0; i < doc.Size(); i++) {
        LoggerPassport item;
        rapidjson::Value &jPlume = doc[i];
        if (!jPlume.IsObject())
            return ERR_LOGGER_PASSPORT_INVALID_JSON;
        if (jPlume.HasMember(JSON_PASSPORT_NAMES[JSON_MODIFIED])) {
            rapidjson::Value &jModified = jPlume[JSON_PASSPORT_NAMES[JSON_MODIFIED]];
            if (jModified.IsUint())
                item.modificationTime = jModified.GetUint();
        }
        if (jPlume.HasMember(JSON_PASSPORT_NAMES[JSON_NAME])) {
            rapidjson::Value &jName = jPlume[JSON_PASSPORT_NAMES[JSON_NAME]];
            if (jName.IsString())
                item.name = jName.GetString();
        }
        if (jPlume.HasMember(JSON_PASSPORT_NAMES[JSON_ID])) {
            rapidjson::Value &jId = jPlume[JSON_PASSPORT_NAMES[JSON_ID]];
            if (jId.IsObject()) {
                if (jId.HasMember(JSON_PASSPORT_NAMES[JSON_ID_PLUME])) {
                    rapidjson::Value &jIdPlume = jId[JSON_PASSPORT_NAMES[JSON_ID_PLUME]];
                    if (jIdPlume.IsUint())
                        item.id.plume = jIdPlume.GetUint();
                }
                if (jId.HasMember(JSON_PASSPORT_NAMES[JSON_ID_YEAR])) {
                    rapidjson::Value &jIdYear = jId[JSON_PASSPORT_NAMES[JSON_ID_YEAR]];
                    if (jIdYear.IsUint())
                        item.id.year = jIdYear.GetUint();
                }
            }
        }
        // sensors
        if (jPlume.HasMember(JSON_PASSPORT_NAMES[JSON_SENSORS])) {
            rapidjson::Value &jSensors = jPlume[JSON_PASSPORT_NAMES[JSON_SENSORS]];
            if (jSensors.IsArray()) {
                for (int s = 0; s < jSensors.Size(); s++) {
                    rapidjson::Value &jSensor = jSensors[s];
                    SensorCoefficients sensor;
                    if (jSensor.HasMember(JSON_PASSPORT_NAMES[JSON_SENSORS_MAC])) {
                        rapidjson::Value &jSensorMac = jSensor[JSON_PASSPORT_NAMES[JSON_SENSORS_MAC]];
                        if (jSensorMac.IsString()) {
                            sensor.mac.set(jSensorMac.GetString());
                        }
                    }
                    if (jSensor.HasMember(JSON_PASSPORT_NAMES[JSON_SENSORS_COEFFICIENTS])) {
                        rapidjson::Value &jSensorCoefficients = jSensor[JSON_PASSPORT_NAMES[JSON_SENSORS_COEFFICIENTS]];
                        if (jSensorCoefficients.IsArray()) {
                            for (int c = 0; c < jSensorCoefficients.Size(); c++) {
                                rapidjson::Value &jCoefficient = jSensorCoefficients[c];
                                if (jCoefficient.IsDouble()) {
                                    sensor.coefficients.push_back(jCoefficient.GetDouble());
                                }
                            }
                        }
                    }
                    item.sensors.push_back(sensor);
                }
            }
        }
        this->push(item);
    }
    return 0;
}

int LoggerPassportCollection::parseText(
	time_t modificationTime,
    const std::string &name, ///< optional resource name
	std::istream &strm
)
{
	PASSPORT_TOKEN token = TOK_YEAR_PLUME;
	LoggerPassport item;
	SensorCoefficients sensor;

	int r = 0;
	int sensorCount = 0;
	while (!strm.eof()) {
		std::string value = next(strm);
		// fix
		if (token == TOK_COEFF) {
			if (value.size()) {
				if (value[0] == 'N') {
					token = TOK_YEAR_PLUME;
				}
				else {
					if (isHex(value)) {	// is hex number
						token = TOK_MAC;
					}
				}
			}
		}
		switch (token)
		{
		case TOK_YEAR_PLUME:
			if (sensorCount) {
				item.sensors.push_back(sensor);
				sensorCount = 0;
			}
			if (r)
				push(item);
            if (putYearPlume(item, value)) {
                item.sensors.clear();
                item.modificationTime = modificationTime;
                item.name = name;     // optional resource name
                token = TOK_MAC;
                r++;
            } else
                token = TOK_END;
			break;
		case TOK_MAC:
			if (sensorCount) {
				item.sensors.push_back(sensor);
			}
			if (!putMac(sensor, value))
				token = TOK_END;
			else {
				sensorCount++;
				token = TOK_COEFF;
			}
			sensor.coefficients.clear();
			break;
		case TOK_COEFF:
			if (!putCoefficient(sensor, value))
				token = TOK_END;
			else {
				token = TOK_COEFF;
			}
			break;
		default:	// TOK_END
			break;
		}
		if (token == TOK_END)
			break;
	}
	return r;
}

int LoggerPassportCollection::loadString(
	time_t modificationTime,
	const std::string &value,
    const bool isJson
)
{
	std::stringstream ss(value);
    if (isJson)
        return parseJson(modificationTime, noname, ss);
    else
	    return parseText(modificationTime, noname, ss);
}

/**
 * @brief List files in the directory
 * @param retval file list
 * @param path Initial path. List all subdirectories in specified path.
 * @param fileSuffix Search files with file extension e.g. ".txt"
 * @return count of files, <0- error
 */
static size_t listDir(
        std::vector<std::string> &retval,
        const std::string &path,
        const std::string &fileSuffix
)
{
    return listFiles(&retval, path, fileSuffix, 1);
}

int LoggerPassportCollection::loadFile(
	const std::string &fileName,
	const std::string &fileSuffix
)
{
	time_t m;
	if (!isOrdinalFile(m, fileName.c_str())) {
		std::vector<std::string> fileNames;
		listDir(fileNames, fileName, fileSuffix);
		return loadFiles(fileNames, fileSuffix);
	}
	std::ifstream f(fileName, std::ifstream::in | std::ifstream::binary);
	if (f.bad()) {
		f.close();
		return 0;
	}
    int r;
    if (fileIsJSON(fileName))
	    r = parseJson(m, fileName, f);
    else
        r = parseText(m, fileName, f);
	f.close();
	return r;
}

int LoggerPassportCollection::loadFiles(
	const std::vector<std::string> &fileNames,
	const std::string &fileSuffix
)
{
	int r = 0;
	for (std::vector<std::string>::const_iterator it(fileNames.begin()); it != fileNames.end(); it++) {
		r = loadFile(*it, fileSuffix);
		if (r < 0)
			break;
	}
	return r;
}

// -------- LoggerPassportMemory --------

LoggerPassportMemory::LoggerPassportMemory()
{

}

LoggerPassportMemory::LoggerPassportMemory(const LoggerPassportMemory& value)
	: values(value.values)
{

}

 LoggerPassportMemory::~LoggerPassportMemory()
 {

 }

 size_t LoggerPassportMemory::count() const
 {
	return values.size();
 }

const LoggerPassport *LoggerPassportMemory::get(const LoggerPlumeId &id) const
{
	std::map <LoggerPlumeId, LoggerPassport>::const_iterator it(values.find(id));
	if (it == values.end())
		return nullptr;
	return &it->second;
}

size_t LoggerPassportMemory::ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const 
{
	size_t ofs = 0;
	size_t cnt = 0;
	for (std::map <LoggerPlumeId, LoggerPassport>::const_iterator it(values.begin()); it != values.end(); it++) {
		ofs++;
		if (ofs <= offset)
			continue;
		cnt++;
		if (limit > 0) {
			if (cnt > limit)
				break;
		}
		retval.push_back(it->first);
	}
	return cnt;
}

void LoggerPassportMemory::push(LoggerPassport &value)
{
	values[value.id] = value;
}

void LoggerPassportMemory::remove(
    const LoggerPlumeId &id
)
{
    std::map <LoggerPlumeId, LoggerPassport>::iterator it(values.find(id));
    if (it != values.end())
        values.erase(it);
}
