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
void Split(const std::string& strVal, int32_t outsize, uint64_t *outVals, uint64_t nDefault);
std::string GetArg(const std::string& strArg, const std::string& strDefault);
int64_t GetArg(const std::string& strArg, int64_t nDefault);
bool GetBoolArg(const std::string& strArg, bool fDefault);

void LogPrintf(const char* format, ...);
void StartShutdown(int errorCode = EXIT_FAILURE);

void calc_rmd160_sha256(uint8_t rmd160[20],uint8_t *data,int32_t datalen);

#define ASSETCHAINS_N 96
#define ASSETCHAINS_K 5

#ifndef SATOSHIDEN
#define SATOSHIDEN ((uint64_t)100000000L)
#define dstr(x) ((double)(x) / SATOSHIDEN)
#endif

void komodo_args(char *argv0);

#endif // KOMODO_UTILS_H
