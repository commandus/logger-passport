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
        << ", \"plume\": " << this->plume.toJsonString()
		<< "}";
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

int LoggerPassportCollection::loadStream(
	time_t modificationTime,
    const std::string &name, ///< optional resource name
	const std::istream &strm
)
{
	LoggerPassportItem it;
	it.modificationTime = modificationTime;
	it.id.plume = plume1;
    it.name = name;     // optional resource name
	plume1++;
	it.id.year = 2022;
	push(it);
	return 0;
}

int LoggerPassportCollection::loadString(
	time_t modificationTime,
	const std::string &value
)
{
	std::stringstream ss(value);
	return loadStream(modificationTime, noname, ss);
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
	std::ifstream f(fileName, std::ios::in);
	int r = loadStream(m, fileName, f);
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
		if (r)
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
