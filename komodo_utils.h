#ifndef KOMODO_UTILS_H
#define KOMODO_UTILS_H

#include <map>
#include <vector>
#include <string>

// Extern declarations
extern std::map<std::string, std::string> mapArgs;
extern std::map<std::string, std::vector<std::string>> mapMultiArgs;

// Function declarations
int64_t atoi64(const char* psz);
int64_t atoi64(const std::string& str);
int atoi(const std::string& str);
std::string GetArg(const std::string& strArg, const std::string& strDefault);
int64_t GetArg(const std::string& strArg, int64_t nDefault);
bool GetBoolArg(const std::string& strArg, bool fDefault);

void LogPrintf(const char* format, ...);
void StartShutdown(int errorCode = EXIT_FAILURE);

#endif // KOMODO_UTILS_H
