#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>
#include <cstdint>

using namespace std;

int64_t GetTime()
{
    return time(NULL);
}

std::string DateTimeStrFormat(const char* pszFormat, int64_t nTime)
{
    std::time_t time = static_cast<std::time_t>(nTime);
    std::tm tm = *std::gmtime(&time);  // Convert to GMT/UTC time

    std::stringstream ss;
    ss << std::put_time(&tm, pszFormat);
    return ss.str();
}

bool LogAcceptCategory(const char* category)
{
    return true;
}

/**
 * fStartedNewLine is a state variable held by the calling context that will
 * suppress printing of the timestamp when multiple calls are made that don't
 * end in a newline. Initialize it to true, and hold it, in the calling context.
 */
static std::string LogTimestampStr(const std::string &str, bool *fStartedNewLine)
{
    string strStamped;

    if (*fStartedNewLine)
        strStamped =  DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()) + ' ' + str;
    else
        strStamped = str;

    if (!str.empty() && str[str.size()-1] == '\n')
        *fStartedNewLine = true;
    else
        *fStartedNewLine = false;

    return strStamped;
}

int LogPrintStr(const std::string &str)
{
    int ret = 0; // Returns total number of characters written
    static bool fStartedNewLine = true;
    // print to console
    ret = fwrite(str.data(), 1, str.size(), stderr);
    fflush(stderr);
    return ret;
}
