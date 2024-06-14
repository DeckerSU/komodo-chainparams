#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include "assetchain.h"
#include "komodo_utils.h"
#include "komodo_globals.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

static void InterpretNegativeSetting(std::string name, std::map<std::string, std::string>& mapSettingsRet)
{
    // interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
    if (name.find("-no") == 0)
    {
        std::string positive("-");
        positive.append(name.begin()+3, name.end());
        if (mapSettingsRet.count(positive) == 0)
        {
            bool value = !GetBoolArg(name, false);
            mapSettingsRet[positive] = (value ? "1" : "0");
        }
    }
}

void ParseParameters(int argc, const char* const argv[])
{
    mapArgs.clear();
    mapMultiArgs.clear();

    for (int i = 1; i < argc; i++)
    {
        std::string str(argv[i]);
        std::string strValue;
        size_t is_index = str.find('=');
        if (is_index != std::string::npos)
        {
            strValue = str.substr(is_index + 1);
            str = str.substr(0, is_index);
        }
#ifdef _WIN32
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        if (str[0] == '/')
            str = "-" + str.substr(1);
#endif

        if (str[0] != '-')
            break;

        // Interpret --foo as -foo.
        // If both --foo and -foo are set, the last takes effect.
        if (str.length() > 1 && str[1] == '-')
            str = str.substr(1);

        mapArgs[str] = strValue;
        mapMultiArgs[str].push_back(strValue);
    }

    // New 0.6 features:
    for (const auto& entry : mapArgs)
    {
        // interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
        InterpretNegativeSetting(entry.first, mapArgs);
    }
}

void PrintMapArgs()
{
    std::cerr << "mapArgs:" << std::endl;
    for (const auto& entry : mapArgs)
    {
        std::cerr << entry.first << " = " << entry.second << std::endl;
    }
}

void PrintMapMultiArgs()
{
    std::cerr << "mapMultiArgs:" << std::endl;
    for (const auto& entry : mapMultiArgs)
    {
        std::cerr << entry.first << " = [ ";
        for (const auto& value : entry.second)
        {
            std::cerr << value << " ";
        }
        std::cerr << "]" << std::endl;
    }
}

std::string magicToHex(uint32_t magic) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << magic;
    return ss.str();
}

uint32_t swapBytes(uint32_t num) {
    uint32_t byte0 = (num & 0x000000FF) << 24;
    uint32_t byte1 = (num & 0x0000FF00) << 8;
    uint32_t byte2 = (num & 0x00FF0000) >> 8;
    uint32_t byte3 = (num & 0xFF000000) >> 24;
    return (byte0 | byte1 | byte2 | byte3);
}

extern assetchain chainName;

int main(int argc, char **argv) {
    std::cerr << "komodo-chainparams (c) Decker" << std::endl;

    // ParseParameters(argc, argv.get()); // before calling komodo_args -ac_name param should be set in mapArgs
    // komodo_args(argv0Data.get());      // argv0 is passed in try to get ac_name from program suffixes (works for MNZ and BTCH only)
    // chainparams_commandline();         // set CChainParams (pCurrentParams) from ASSETCHAINS_* global variables

    ParseParameters(argc, argv);
    PrintMapArgs();
    PrintMapMultiArgs();
    komodo_args(argv[0]);
    // chainparams_commandline();

    json j;
    j["chainname"] = chainName.ToString();
    j["p2pport"] = ASSETCHAINS_P2PPORT;
    j["rpcport"] = ASSETCHAINS_RPCPORT;
    j["magic"] = ASSETCHAINS_MAGIC;
    j["magic_hex"] = magicToHex(ASSETCHAINS_MAGIC);
    j["magic_bytes"] = magicToHex(swapBytes(ASSETCHAINS_MAGIC));
    
    std::string s = j.dump();
    std::cout << s << std::endl;

    return 0;
}