#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <rapidjson/document.h>

#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>

#include "logger-plume-collection.h"
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
    ss << mac.toString();
    bool isFirstLine = true;
    for (auto it(coefficients.begin()); it != coefficients.end(); it++) {
        if (isFirstLine)
            isFirstLine = false;
        else
            ss << "\t\t"; // add extra tab
        for (auto it2(it->begin()); it2 != it->end(); it2++) {
            ss << "\t" << std::fixed << std::setprecision(5) << *it2;
        }
        ss << std::endl;
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

        ss << "[";
        bool isFirst2 = true;
        for (auto it2(it->begin()); it2 != it->end(); it2++) {
            if (isFirst2)
                isFirst2 = false;
            else
                ss << COMMA;
            ss << std::fixed << std::setprecision(5) << *it2;
        }
        ss << "]";
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
) const
{
    int idx = 0;
    int t = temperature;
    int sz = coefficients.size();
    if (sz == 3) {
        if (t < -3)
            idx = 0;
        else if (t <= 3)
            idx = 1;
        else if (t <= 30)
            idx = 2;
        else
            return temperature;
    } else {
        if (sz >= 1)
            idx = 0;
        else
            return temperature;
    }
    int sz2 = coefficients[idx].size();
    switch (sz2) {
        case 0:
        case 1:
            break;
        case 2:
            return coefficients[idx][0] * temperature + coefficients[idx][1];
        default:    // 3
        {
            double x = 1.0;
            double r = 0.0;
            for (int i = 0; i < sz2; i++) {
                r += coefficients[idx][i] * x;
                if (i == 0)
                    x = temperature;
                else
                    x *= x;
            }
            return r;
        }
    }
    return temperature;
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

// -------- LoggerPlume --------

const static std::string noname;

LoggerPlume::LoggerPlume()
	: modificationTime(0), name(noname)
{
}

LoggerPlume::LoggerPlume(
	const LoggerPlume& value
)
	: modificationTime(value.modificationTime), id(value.id), name(value.name)
{

}

LoggerPlume::LoggerPlume(
	time_t aModificationTime
)
	: modificationTime(aModificationTime), name(noname)
{

}

LoggerPlume::~LoggerPlume()
{

}

std::string LoggerPlume::toJsonString() const
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

std::string LoggerPlume::toString() const
{
	std::stringstream ss;
	ss << id.toString() << std::endl;
    for (std::vector<SensorCoefficients>::const_iterator it(sensors.begin()); it != sensors.end(); it++) {
        ss << it->toString();
    }
	return ss.str();
}

std::string LoggerPlume::toTableString() const
{
	return toString();
}

std::string LoggerPlume::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toString();
}

// -------- LoggerPlumeCollection --------

std::string LoggerPlumeCollection::toJsonString() const
{
	std::vector<LoggerPlumeId> plumeIds;
	size_t cnt = ids(plumeIds, 0, 0);
	std::stringstream ss;
	ss << "[";
	bool firstItem = true;
	for (std::vector<LoggerPlumeId>::const_iterator it(plumeIds.begin()); it != plumeIds.end(); it++) {
		const LoggerPlume *v = get(*it);
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

std::string LoggerPlumeCollection::toString() const
{
    std::vector<LoggerPlumeId> plumeIds;
    size_t cnt = count();
    ids(plumeIds, 0, cnt);
    std::stringstream ss;
    for (std::vector<LoggerPlumeId>::const_iterator it(plumeIds.begin()); it != plumeIds.end(); it++) {
        const LoggerPlume *p = get(*it);
        if (p)
            ss << p->toString();
    }
    ss << "NEND" << std::endl;
	return ss.str();
}

std::string LoggerPlumeCollection::toTableString() const
{
	return toJsonString();
}

std::string LoggerPlumeCollection::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toJsonString();
}

const LoggerPlume *LoggerPlumeCollection::get(int serialNo, int year) const
{
    return get(LoggerPlumeId(serialNo, year));
}

const SensorCoefficients *LoggerPlumeCollection::getSensor(int serialNo, int year, uint64_t mac) const
{
    const LoggerPlume *p = get(LoggerPlumeId(serialNo, year));
    if (!p)
        return nullptr;
    for (std::vector<SensorCoefficients>::const_iterator it(p->sensors.begin()); it != p->sensors.end(); it++) {
        if (it->mac == mac)
            return &*it;
    }
    return nullptr;
}

/**
 * Remove passport form the storage
 * @param id plume identifier
 */
void LoggerPlumeCollection::remove(
    int serialNo,
    int year
)
{
    remove(LoggerPlumeId(serialNo, year));
}

double LoggerPlumeCollection::calc(
    int serialNo,
    int year,
    uint64_t sensor,
    double temperature
) const
{
    const SensorCoefficients *c = getSensor(serialNo, year, sensor);
    if (c)
        return c->calc(temperature);
    return temperature;
}

double LoggerPlumeCollection::calc(
    uint64_t mac,
    double temperature
) const
{
    const SensorCoefficients *c = getSensor(mac);
    if (c)
        return c->calc(temperature);
    return temperature;
}

static void skipComments(
	std::istream& strm
)
{
	while (!strm.eof()) {
		char c;
		strm.get(c);
		if (strm.bad())
			break;
		if (c == 10)
			break;
	}
}

/**
 * Return next token. retNewLine indicate is token at new line
 * @param retNewLine return if token in a new line (coefficients in a new line means a new temperature range)
 * @param strm
 * @return token
 */
static std::string next(
    bool &retNewLineBefore,
    bool &retNewLineAfter,
	std::istream& strm
)
{
    retNewLineBefore = false;
    retNewLineAfter = false;
	std::stringstream ss;
	char c;
	// skip spaces
	while (!strm.eof()) {
		if (!strm.get(c))
			break;
		if (c <= 32) {
            if (c == 10)
                retNewLineBefore = true;
            continue;
        }
		if (c == '#') {
			skipComments(strm);
            continue;
        }
		ss << c;
		break;
	}
	
	while (!strm.eof()) {
		if (!strm.get(c))
			break;
		if (c <= 32) {
            if (c == 10)
                retNewLineAfter = true;
            break;
        }
		if (c == '#') {
			skipComments(strm);
            continue;
        }
		ss << c;
	}
	return ss.str();
}

static bool putYearPlume(
        LoggerPlume &retVal,
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
    retval.coefficients.back().push_back(c);
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

int LoggerPlumeCollection::parseJson(
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
        LoggerPlume item;
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
                                std::vector <double> a;
                                sensor.coefficients.push_back(a);
                                rapidjson::Value &jCoefficient1 = jSensorCoefficients[c];
                                if (jCoefficient1.IsArray()) {
                                    for (int c1 = 0; c1 < jCoefficient1.Size(); c1++) {
                                        rapidjson::Value &jCoefficient2 = jCoefficient1[c1];
                                        if (jCoefficient2.IsDouble()) {
                                            sensor.coefficients.back().push_back(jCoefficient2.GetDouble());
                                        }
                                    }
                                }
                                rapidjson::Value &jCoefficient = jSensorCoefficients[c];
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

int LoggerPlumeCollection::parseText(
	time_t modificationTime,
    const std::string &name, ///< optional resource name
	std::istream &strm
)
{
	PASSPORT_TOKEN token = TOK_YEAR_PLUME;
	LoggerPlume item;
	SensorCoefficients sensor;

	int r = 0;
	int sensorCount = 0;
    
    bool tokenAtNewLineAfterEarlier = false;
	while (!strm.eof()) {
        bool tokenAtNewLineBefore;
        bool tokenAtNewLineAfter;
		std::string value = next(tokenAtNewLineBefore, tokenAtNewLineAfter, strm);
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
            if ((token == TOK_COEFF) && (tokenAtNewLineBefore || tokenAtNewLineAfterEarlier)) {
                std::vector <double> v;
                v.reserve(3);
                sensor.coefficients.push_back(v);
            }
            tokenAtNewLineAfterEarlier = tokenAtNewLineAfter;
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
                // clear old ..
                sensor.coefficients.clear();
                // and reserve one polynomial
                std::vector <double> v;
                v.reserve(3);
                sensor.coefficients.push_back(v);
				sensorCount++;
                tokenAtNewLineAfterEarlier = false;
				token = TOK_COEFF;
			}
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

int LoggerPlumeCollection::loadString(
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

int LoggerPlumeCollection::loadFile(
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

int LoggerPlumeCollection::loadFiles(
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

// -------- LoggerPlumeMemory --------

LoggerPlumeMemory::LoggerPlumeMemory()
{

}

LoggerPlumeMemory::LoggerPlumeMemory(const LoggerPlumeMemory& value)
	: values(value.values)
{

}

 LoggerPlumeMemory::~LoggerPlumeMemory()
 {

 }

 size_t LoggerPlumeMemory::count() const
 {
	return values.size();
 }

const LoggerPlume *LoggerPlumeMemory::get(const LoggerPlumeId &id) const
{
    if (!mapMutex.try_lock())
        return nullptr;
	std::map <LoggerPlumeId, LoggerPlume>::const_iterator it(values.find(id));
    mapMutex.unlock();
	if (it == values.end())
		return nullptr;
	return &it->second;
}

const SensorCoefficients *LoggerPlumeMemory::getSensor(
    uint64_t mac
) const
{
    if (!mapMutex.try_lock())
        return nullptr;
    std::map <uint64_t, const SensorCoefficients *>::const_iterator it(macIndex.find(mac));        
    mapMutex.unlock();
    if (it != macIndex.end())
        return it->second;
    return nullptr;        
}

size_t LoggerPlumeMemory::ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const
{
	size_t ofs = 0;
	size_t cnt = 0;
    for (std::map <LoggerPlumeId, LoggerPlume>::const_iterator it(values.begin()); it != values.end(); it++) {
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

void LoggerPlumeMemory::push(LoggerPlume &value)
{
	values[value.id] = value;
}

void LoggerPlumeMemory::remove(
    const LoggerPlumeId &id
)
{
    std::map <LoggerPlumeId, LoggerPlume>::iterator it(values.find(id));
    if (it != values.end())
        values.erase(it);\
}

void LoggerPlumeMemory::clear()
{
    values.clear();
    macIndex.clear();
}

void LoggerPlumeMemory::startModification()
{
    mapMutex.lock();
}

void LoggerPlumeMemory::finishModification()
{
    buildMacIndex();
    mapMutex.unlock();
}

void LoggerPlumeMemory::buildMacIndex()
{
    macIndex.clear();
    for (std::map <LoggerPlumeId, LoggerPlume>::const_iterator it(values.begin()); it != values.end(); it++) {
        for (std::vector <SensorCoefficients>::const_iterator its(it->second.sensors.begin()); its != it->second.sensors.end(); its++) {
            macIndex[its->mac.mac.u] = &*its;
        }
    }
}
