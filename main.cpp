#include <iostream>
#include <map>
#include <vector>

#include "komodo_utils.h"

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
    std::cout << "mapArgs:" << std::endl;
    for (const auto& entry : mapArgs)
    {
        std::cout << entry.first << " = " << entry.second << std::endl;
    }
}

void PrintMapMultiArgs()
{
    std::cout << "mapMultiArgs:" << std::endl;
    for (const auto& entry : mapMultiArgs)
    {
        std::cout << entry.first << " = [ ";
        for (const auto& value : entry.second)
        {
            std::cout << value << " ";
        }
        std::cout << "]" << std::endl;
    }
}

int main(int argc, char **argv) {
    std::cout << "komodo-chainparams (c) Decker" << std::endl;

    // ParseParameters(argc, argv.get()); // before calling komodo_args -ac_name param should be set in mapArgs
    // komodo_args(argv0Data.get());      // argv0 is passed in try to get ac_name from program suffixes (works for MNZ and BTCH only)
    // chainparams_commandline();         // set CChainParams (pCurrentParams) from ASSETCHAINS_* global variables

    ParseParameters(argc, argv);
    PrintMapArgs();
    PrintMapMultiArgs();
    // komodo_args(argv[0]);
    // chainparams_commandline();


    return 0;
}