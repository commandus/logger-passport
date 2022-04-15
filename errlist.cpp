#include <sstream>
#include "errlist.h"

static const char *unknown_error_en = "Unknown error";

static const char *error_list_en[ERR_LIST_COUNT] =
{
	"Segmentation fault",
	"Program aborted",
	"Invalid command line option",
	"Help screen",
	"Insufficient memory",
	"Insufficient parameters",
	"Invalid parameter",
	"No data",
	"Invalid packet or no packet"
};

const char *strerror_logger_passport(
	int errCode
)
{
	int idx = (- errCode) - 710;
	if (idx < 0 || idx >= ERR_LIST_COUNT) {
		return unknown_error_en;
	}
	return error_list_en[idx];
}
