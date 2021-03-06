#ifndef ERRLIST_H
#define ERRLIST_H     1

// Error codes
#define LOGGER_PASSPORT_OK 						0
#define	ERR_LIST_COUNT							10
#define ERR_LOGGER_PASSPORT_SEGMENTATION_FAULT	-710
#define ERR_LOGGER_PASSPORT_ABRT           		-711
#define ERR_LOGGER_PASSPORT_COMMAND_LINE   		-712
#define ERR_LOGGER_PASSPORT_COMMAND_LINE_HELP	-713
#define ERR_LOGGER_PASSPORT_INSUFFICIENT_MEMORY	-714
#define ERR_LOGGER_PASSPORT_INSUFFICIENT_PARAMS	-715
#define ERR_LOGGER_PASSPORT_PARAM_INVALID		-716
#define ERR_LOGGER_PASSPORT_NO_DATA      		-717
#define ERR_LOGGER_PASSPORT_INVALID_FORMAT		-718
#define ERR_LOGGER_PASSPORT_INVALID_JSON        -719

#define ERR_MESSAGE								"Error "
#define ERR_DEBUG								"Info "
#define ERR_WARNING								"Warning "
#define ERR_UNKNOWN_ERROR_CODE					"Unknown error code "

const char *strerror_logger_passport(
	int errCode
);

#endif
