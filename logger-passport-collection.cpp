#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "logger-passport-collection.h"
#include "list-file.h"

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

std::string SensorMAC::toJsonString() const
{
    return toString();
}

std::string SensorMAC::toTableString() const
{
    return toString();
}

std::string SensorMAC::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
    return toString();
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
    for (auto it(coefficents.begin()); it != coefficents.end(); it++) {
        ss << std::fixed << std::setprecision(10) << *it << " ";
    }
    return ss.str();
}

std::string SensorCoefficients::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"mac\": \"" << mac.toString() << "\", \"coefficients\": [";
    bool isFirst = true;
    for (auto it(coefficents.begin()); it != coefficents.end(); it++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << std::fixed << std::setprecision(10) << *it;
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
	int aplume,
	int ayear
)
	: plume(aplume), year(ayear)
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
    // std::cerr << toJsonString() << " < " << another.toJsonString()<< std::endl;
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
		<< "\"plume\": " << plume
		<< ", \"year\": " << year
		<< "}";
	return ss.str();
}

std::string LoggerPlumeId::toString() const
{
	std::stringstream ss;
	ss << plume << "-" << year;
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

// -------- LoggerPassportItem --------

const static std::string noname;

LoggerPassportItem::LoggerPassportItem()
	: modificationTime(0), name(noname)
{
}

LoggerPassportItem::LoggerPassportItem(
	const LoggerPassportItem& value
)
	: modificationTime(value.modificationTime), id(value.id), name(value.name)
{

}

LoggerPassportItem::LoggerPassportItem(
	time_t amodificationTime
)
	: modificationTime(amodificationTime), name(noname)
{

}

LoggerPassportItem::~LoggerPassportItem()
{

}

std::string LoggerPassportItem::toJsonString() const
{
	std::stringstream ss;
	ss << "{"
		<< "\"modified\": " << modificationTime
		<< ", \"name\": \"" << name
		<< "\", \"id\": " << id.toJsonString()
		<< ", \"plume\": [";
	bool isFirst = true;
	for (auto it(sensors.begin()); it != sensors.end(); it++) {
		if (isFirst)
			isFirst = false;
		else
			ss << ", ";
		ss << it->toJsonString();
	}
	ss << "]}";
	return ss.str();
}

std::string LoggerPassportItem::toString() const
{
	std::stringstream ss;
	ss << id.toString() << " " << modificationTime;
	return ss.str();
}

std::string LoggerPassportItem::toTableString() const
{
	return toString();
}

std::string LoggerPassportItem::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
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
		const LoggerPassportItem *v = get(*it);
		if (!v)
			continue;
		if (firstItem)
			firstItem = false;
		else
			ss << ", ";
		ss << v->toJsonString();
	}
	ss << "]";
	return ss.str();
}

std::string LoggerPassportCollection::toString() const
{
	return toJsonString();
}

std::string LoggerPassportCollection::toTableString() const
{
	return toJsonString();
}

std::string LoggerPassportCollection::sqlInsertPackets(LOGGER_OUTPUT_FORMAT outputFormat) const
{
	return toJsonString();
}

static int plume1 = 0;

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
		char c;
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
	LoggerPassportItem &retval,
	const std::string &value
)
{
	size_t p = value.find('-');
	if (p == std::string::npos)
		return false;
	std::string sYear = value.substr(1, p);
	retval.id.year = strtol(sYear.c_str(), NULL, 10);
	std::string sPlume= value.substr(p + 1);
	retval.id.plume = strtol(sPlume.c_str(), NULL, 10);
	return true;
}

static bool putMac(
	SensorCoefficients &retval,
	const std::string& value
)
{
	uint64_t m = strtoull(value.c_str(), NULL, 16);
	if (!m)
		return false;
	retval.mac = m;
	
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
	double c = strtod(value.c_str(), NULL);
	retval.coefficents.push_back(c);
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

int LoggerPassportCollection::parse(
	time_t modificationTime,
    const std::string &name, ///< optional resource name
	std::istream &strm
)
{
	PASSPORT_TOKEN token = TOK_YEAR_PLUME;
	LoggerPassportItem item;
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
			else {
				if (!putYearPlume(item, value))
					token = TOK_END;
				else {
					item.modificationTime = modificationTime;
					item.name = name;     // optional resource name
					token = TOK_MAC;
					r++;
				}
			}
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
			sensor.coefficents.clear();
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
	const std::string &value
)
{
	std::stringstream ss(value);
	return parse(modificationTime, noname, ss);
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
	int r = parse(m, fileName, f);
	f.close();
	return r;
}

size_t LoggerPassportCollection::listDir(
	std::vector<std::string> &retval,
	const std::string &path,
	const std::string &fileSuffix
)
{
	return listFiles(&retval, path, fileSuffix, 1);
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

const LoggerPassportItem *LoggerPassportMemory::get(const LoggerPlumeId &id) const 
{
	std::map <LoggerPlumeId, LoggerPassportItem>::const_iterator it(values.find(id));
	if (it == values.end())
		return NULL;
	return &it->second;
}

size_t LoggerPassportMemory::ids(std::vector<LoggerPlumeId> &retval, size_t offset, size_t limit) const 
{
	size_t ofs = 0;
	size_t cnt = 0;
	for (std::map <LoggerPlumeId, LoggerPassportItem>::const_iterator it(values.begin()); it != values.end(); it++) {
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

void LoggerPassportMemory::push(LoggerPassportItem &value)
{
	values[value.id] = value;
}
