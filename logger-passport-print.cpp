/**
 * @brief Logger passport passport print utility
 * @file logger-passport-print.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "argtable3/argtable3.h"

#include "logger-plume-collection.h"
#include "errlist.h"

const static std::string programName = "logger-passport-print";

#define DEF_PASSPORT_FILE_SUFFIX    ".txt"

typedef enum {
    LOGGER_INPUT_SRC_STDIN = 0,                 ///< read from stdin
    LOGGER_INPUT_SRC_FILES = 1                  ///< read from files
} LOGGER_INPUT_SOURCE;

static const std::string SQL_DIALECT_NAME[] = {
    "postgresql", "mysql", "firebird", "sqlite"
};

class LoggerPassportPrintConfiguration {
public:
    std::vector<std::string> fileNames;         ///< file names
    LOGGER_INPUT_SOURCE mode;                   ///< LOGGER_INPUT_SRC_STDIN(0)- from stdin, LOGGER_INPUT_SRC_FILES(1)- from files
    LOGGER_OUTPUT_FORMAT outputFormat;          ///< default 0- JSON
    bool calculate;                             ///< true- calculate temperature
    double temperature;
    std::vector<uint64_t> macs;
    int verbosity;                              ///< verbosity level

    LoggerPassportPrintConfiguration();
};
LoggerPassportPrintConfiguration::LoggerPassportPrintConfiguration()
    : mode(LOGGER_INPUT_SRC_STDIN), outputFormat(LOGGER_OUTPUT_FORMAT_JSON),
    verbosity(0)
{

}

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
        LoggerPassportPrintConfiguration *config,
        int argc,
        char *argv[])
{
    struct arg_str *a_filenames = arg_strn(nullptr, nullptr, "<file>", 0, 100, "Passport file names");
    struct arg_lit *a_stdin = arg_lit0("r", "read", "Read from stdin");
    struct arg_str *a_output_format = arg_str0("f", "format", "json|text|table|postgresql|mysql|firebird|sqlite", "Default json");

    struct arg_dbl *a_calc = arg_dbl0("c", "calc", "<temperature>", "e.g. -12.3");
    struct arg_str *a_macs = arg_strn("m", "mac", "<hex>", 0, 100, "calc MAC");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_filenames, a_stdin, a_output_format,
        a_calc, a_macs,
        a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    if ((nerrors == 0) && (a_help->count == 0)) {
        config->outputFormat = LOGGER_OUTPUT_FORMAT_JSON;
        if (a_output_format->count) {
            std::string s(*a_output_format->sval);
            if (s == "text")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_TEXT;
            if (s == "table")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_TABLE;
            if (s == SQL_DIALECT_NAME[0])
                config->outputFormat = LOGGER_OUTPUT_FORMAT_PG;
            if (s == SQL_DIALECT_NAME[1])
                config->outputFormat = LOGGER_OUTPUT_FORMAT_MYSQL;
            if (s == SQL_DIALECT_NAME[2])
                config->outputFormat = LOGGER_OUTPUT_FORMAT_FB;
            if (s == SQL_DIALECT_NAME[3])
                config->outputFormat = LOGGER_OUTPUT_FORMAT_SQLITE;
        }
        config->verbosity = a_verbosity->count;
        for (int i = 0; i < a_filenames->count; i++) {
            config->fileNames.push_back(a_filenames->sval[i]);
        }
        if (a_stdin->count)
            config->mode = LOGGER_INPUT_SRC_STDIN;
        else {
            config->mode = LOGGER_INPUT_SRC_FILES;
            if (config->fileNames.size() == 0)
                nerrors++;
        }

        config->calculate = a_calc->count > 0;
        if (config->calculate) {
            config->temperature = *a_calc->dval;
            for (int i = 0; i < a_macs->count; i++) {
                uint64_t m = std::stoull(*a_macs[i].sval, 0, 16);
                config->macs.push_back(m);
            }
        }
    }

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
            << "Print logger passport" << std::endl
            << "  logger-passport-print <file1> <file2> <dir1> ..." << std::endl;

        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

        return ERR_LOGGER_PASSPORT_COMMAND_LINE_HELP;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}

static void printErrorAndExit(
    int errCode
)
{
    std::cerr << ERR_MESSAGE << errCode << ": " << strerror_logger_passport(errCode) << std::endl;
    exit(errCode);
}

const char* TAB_DELIMITER = "\t";

int main(int argc, char **argv)
{
    LoggerPassportPrintConfiguration config;
    int r = parseCmd(&config, argc, argv);
    if (r == ERR_LOGGER_PASSPORT_COMMAND_LINE_HELP)
        exit(r);
    if (r != 0)
        printErrorAndExit(r);

    LoggerPlumeCollection *c = new LoggerPlumeMemory();

    if (config.mode == LOGGER_INPUT_SRC_STDIN)
        c->parseText(time(nullptr), "cin", std::cin);
    else
        c->loadFiles(config.fileNames, DEF_PASSPORT_FILE_SUFFIX);
    // printErrorAndExit(ERR_LOGGER_PASSPORT_INVALID_FORMAT);
    std::string s;
    if (config.calculate) {
        std::stringstream ss;
        bool isFirst = true;
        for (auto it(config.macs.begin()); it != config.macs.end(); it++) {
            if (isFirst)
                isFirst = false;
            else
                ss << "\t";
            double t = c->calc(*it, config.temperature);
            ss << std::fixed << std::setprecision(5) << t;
        }
        s = ss.str();
    } else {
        // list
        switch (config.outputFormat) {
            case LOGGER_OUTPUT_FORMAT_JSON:
                s = c->toJsonString();
                break;
            case LOGGER_OUTPUT_FORMAT_TEXT:
                s = c->toString();
                break;
            case LOGGER_OUTPUT_FORMAT_TABLE:
                s = c->toTableString();
                break;
            default: // LOGGER_OUTPUT_FORMAT_PG LOGGER_OUTPUT_FORMAT_MYSQL LOGGER_OUTPUT_FORMAT_FB LOGGER_OUTPUT_FORMAT_SQLITE
                s = c->sqlInsertPackets(config.outputFormat);
        }
    }
    std::cout << s << std::endl;
    delete c;
}
