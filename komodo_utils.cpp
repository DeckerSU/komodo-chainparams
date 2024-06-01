#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <sstream>

#include <inttypes.h>

#include "assetchain.h"
#include "hex.h"
#include "komodo_globals.h"
#include "komodo_utils.h"


std::map<std::string, std::string> mapArgs;
std::map<std::string, std::vector<std::string> > mapMultiArgs;

char *argv0suffix[] =
{
    (char *)"mnzd", (char *)"mnz-cli", (char *)"mnzd.exe", (char *)"mnz-cli.exe", (char *)"btchd", (char *)"btch-cli", (char *)"btchd.exe", (char *)"btch-cli.exe"
};

char *argv0names[] =
{
    (char *)"MNZ", (char *)"MNZ", (char *)"MNZ", (char *)"MNZ", (char *)"BTCH", (char *)"BTCH", (char *)"BTCH", (char *)"BTCH"
};

int64_t atoi64(const char* psz)
{
#ifdef _MSC_VER
    return _atoi64(psz);
#else
    return strtoll(psz, NULL, 10);
#endif
}

int64_t atoi64(const std::string& str)
{
#ifdef _MSC_VER
    return _atoi64(str.c_str());
#else
    return strtoll(str.c_str(), NULL, 10);
#endif
}

int atoi(const std::string& str)
{
    return atoi(str.c_str());
}

void Split(const std::string& strVal, int32_t outsize, uint64_t *outVals, const uint64_t nDefault)
{
    std::stringstream ss(strVal);
    std::vector<uint64_t> vec;

    uint64_t i, nLast, numVals = 0;

    while ( ss.peek() == ' ' )
        ss.ignore();

    while ( ss >> i )
    {
        outVals[numVals] = i;
        numVals += 1;

        while ( ss.peek() == ' ' )
            ss.ignore();
        if ( ss.peek() == ',' )
            ss.ignore();
        while ( ss.peek() == ' ' )
            ss.ignore();
    }

    if ( numVals > 0 )
        nLast = outVals[numVals - 1];
    else
        nLast = nDefault;

    for ( i = numVals; i < outsize; i++ )
    {
        outVals[i] = nLast;
    }
}

std::string GetArg(const std::string& strArg, const std::string& strDefault)
{
    if (mapArgs.count(strArg))
        return mapArgs[strArg];
    return strDefault;
}

int64_t GetArg(const std::string& strArg, int64_t nDefault)
{
    if (mapArgs.count(strArg))
        return atoi64(mapArgs[strArg]);
    return nDefault;
}

bool GetBoolArg(const std::string& strArg, bool fDefault)
{
    if (mapArgs.count(strArg))
    {
        if (mapArgs[strArg].empty())
            return true;
        return (atoi(mapArgs[strArg]) != 0);
    }
    return fDefault;
}

void LogPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void StartShutdown(int errorCode)
{
    // Exit the program immediately with the specified error code
    std::exit(errorCode);
}

int8_t equihash_params_possible(uint64_t n, uint64_t k)
{
    /* To add more of these you also need to edit:
    * equihash.cpp very end of file with the tempate to point to the new param numbers 
    * equihash.h
    *  line 210/217 (declaration of equihash class)
    * Add this object to the following functions: 
    *  EhInitialiseState 
    *  EhBasicSolve
    *  EhOptimisedSolve
    *  EhIsValidSolution
    * Alternatively change ASSETCHAINS_N and ASSETCHAINS_K in komodo_nk.h for fast testing.
    */
    if ( k == 9 && (n == 200 || n == 210) )
        return(0);
    if ( k == 5 && (n == 150 || n == 144 || n == 96 || n == 48) )
        return(0);
    if ( k == ASSETCHAINS_K && n == ASSETCHAINS_N)
        return(0);
    return(-1);
}

void calc_rmd160_sha256(uint8_t rmd160[20],uint8_t *data,int32_t datalen)
{
    CHash160().Write((const unsigned char *)data, datalen).Finalize(rmd160); // SHA-256 + RIPEMD-160
}

void komodo_args(char *argv0)
{
    uint8_t extrabuf[32756];
    std::memset(extrabuf, 0, sizeof(extrabuf));

    uint8_t disablebits[32];
    uint8_t *extraptr=nullptr;
    FILE *fp; 
    uint16_t nonz=0; // keep track of # CCs enabled
    int32_t extralen = 0; 

    const std::string ntz_dest_path = GetArg("-notary", "");
    IS_KOMODO_NOTARY = ntz_dest_path == "" ? 0 : 1;


    STAKED_NOTARY_ID = GetArg("-stakednotary", -1);
    KOMODO_NSPV = GetArg("-nSPV",0);
    memset(disablebits,0,sizeof(disablebits)); // everything enabled
    if ( GetBoolArg("-gen", false) != 0 )
    {
        KOMODO_MININGTHREADS = GetArg("-genproclimit",-1);
    }
    IS_MODE_EXCHANGEWALLET = GetBoolArg("-exchange", false);
    if ( IS_MODE_EXCHANGEWALLET )
        LogPrintf("IS_MODE_EXCHANGEWALLET mode active\n");
    DONATION_PUBKEY = GetArg("-donation", "");
    NOTARY_PUBKEY = GetArg("-pubkey", "");
    IS_KOMODO_DEALERNODE = GetArg("-dealer",0);
    IS_KOMODO_TESTNODE = GetArg("-testnode",0);
    ASSETCHAINS_STAKED_SPLIT_PERCENTAGE = GetArg("-splitperc",0);
    if ( strlen(NOTARY_PUBKEY.c_str()) == 66 )
    {
        decode_hex(NOTARY_PUBKEY33,33,(char *)NOTARY_PUBKEY.c_str());
        USE_EXTERNAL_PUBKEY = 1;
        // if ( !IS_KOMODO_NOTARY )
        // {
        //     // We dont have any chain data yet, so use system clock to guess. 
        //     // I think on season change should reccomend notaries to use -notary to avoid needing this. 
        //     int32_t kmd_season = getacseason(time(NULL));
        //     for (uint16_t i=0; i<64; i++)
        //     {
        //         if ( strcmp(NOTARY_PUBKEY.c_str(),notaries_elected[kmd_season-1][i][1]) == 0 )
        //         {
        //             IS_KOMODO_NOTARY = true;
        //             KOMODO_MININGTHREADS = 1;
        //             mapArgs ["-genproclimit"] = itostr(KOMODO_MININGTHREADS);
        //             STAKED_NOTARY_ID = -1;
        //             LogPrintf("running as notary.%d %s\n",i,notaries_elected[kmd_season-1][i][0]);
        //             break;
        //         }
        //     }
        // }
    }
    if ( STAKED_NOTARY_ID != -1 && IS_KOMODO_NOTARY == true ) {
        LogPrintf( "Cannot be STAKED and KMD notary at the same time!\n");
        StartShutdown();
    }
	std::string name = GetArg("-ac_name","");
    if ( argv0 != 0 )
    {
        int32_t len = (int32_t)strlen(argv0);
        for (unsigned long i=0; i<sizeof(argv0suffix)/sizeof(*argv0suffix); i++)
        {
            int32_t n = (int32_t)strlen(argv0suffix[i]);
            if ( strcmp(&argv0[len - n],argv0suffix[i]) == 0 )
            {
                //LogPrintf("ARGV0.(%s) -> matches suffix (%s) -> ac_name.(%s)\n",argv0,argv0suffix[i],argv0names[i]);
                name = argv0names[i];
                break;
            }
        }
    }
    chainName = assetchain(name);
    KOMODO_STOPAT = GetArg("-stopat",0);
    MAX_REORG_LENGTH = GetArg("-maxreorg",MAX_REORG_LENGTH);
    WITNESS_CACHE_SIZE = MAX_REORG_LENGTH+10;
    ASSETCHAINS_CC = GetArg("-ac_cc",0);
    KOMODO_CCACTIVATE = GetArg("-ac_ccactivate",0);
    ASSETCHAINS_BLOCKTIME = GetArg("-ac_blocktime",60);
    ASSETCHAINS_PUBLIC = GetArg("-ac_public",0);
    ASSETCHAINS_PRIVATE = GetArg("-ac_private",0);
    KOMODO_SNAPSHOT_INTERVAL = GetArg("-ac_snapshot",0);
    Split(GetArg("-ac_nk",""), sizeof(ASSETCHAINS_NK)/sizeof(*ASSETCHAINS_NK), ASSETCHAINS_NK, 0);
    
    // -ac_ccactivateht=evalcode,height,evalcode,height,evalcode,height....
    uint64_t ccEnablesHeight[512];
    memset(ccEnablesHeight, 0, sizeof(ccEnablesHeight));
    Split(GetArg("-ac_ccactivateht",""), sizeof(ccEnablesHeight)/sizeof(*ccEnablesHeight), ccEnablesHeight, 0);
    // fill map with all eval codes and activation height of 0.
    for ( int i = 0; i < 256; i++ )
        mapHeightEvalActivate[i] = 0;
    for ( int i = 0; i < 512; i++ )
    {
        int32_t ecode = ccEnablesHeight[i];
        int32_t ht = ccEnablesHeight[i+1];
        if ( i > 1 && ccEnablesHeight[i-2] == ecode )
            break;
        if ( ecode > 255 || ecode < 0 )
            LogPrintf( "ac_ccactivateht: invalid evalcode.%i must be between 0 and 256.\n", ecode);
        else if ( ht > 0 )
        {
            // update global map. 
            mapHeightEvalActivate[ecode] = ht;
            LogPrintf( "ac_ccactivateht: ecode.%i activates at height.%i\n", ecode, mapHeightEvalActivate[ecode]);
        }
        i++;
    }
    
    if ( (KOMODO_REWIND= GetArg("-rewind",0)) != 0 )
    {
        LogPrintf("KOMODO_REWIND %d\n",KOMODO_REWIND);
    }
    //KOMODO_EARLYTXID = Parseuint256(GetArg("-earlytxid","0").c_str());    
    ASSETCHAINS_EARLYTXIDCONTRACT = GetArg("-ac_earlytxidcontract",0);

    if ( !chainName.isKMD() )
    {
        std::string selectedAlgo = GetArg("-ac_algo", std::string(ASSETCHAINS_ALGORITHMS[0]));

        uint32_t i;
        for ( i = 0; i < ASSETCHAINS_NUMALGOS; i++ )
        {
            if (std::string(ASSETCHAINS_ALGORITHMS[i]) == selectedAlgo)
            {
                ASSETCHAINS_ALGO = i;
                STAKING_MIN_DIFF = ASSETCHAINS_MINDIFF[i];
                // only worth mentioning if it's not equihash
                if (ASSETCHAINS_ALGO != ASSETCHAINS_EQUIHASH)
                    LogPrintf("ASSETCHAINS_ALGO, algorithm set to %s\n", selectedAlgo.c_str());
                break;
            }
        }
        if ( ASSETCHAINS_ALGO == ASSETCHAINS_EQUIHASH && ASSETCHAINS_NK[0] != 0 && ASSETCHAINS_NK[1] != 0 )
        {
            if ( equihash_params_possible(ASSETCHAINS_NK[0], ASSETCHAINS_NK[1]) == -1 ) 
            {
                LogPrintf("equihash values N.%" PRIu64 " and K.%" PRIu64 " are not currently available\n", ASSETCHAINS_NK[0], ASSETCHAINS_NK[1]);
                exit(0);
            } else LogPrintf("ASSETCHAINS_ALGO, algorithm set to equihash with N.%" PRIu64 " and K.%" PRIu64 "\n", ASSETCHAINS_NK[0], ASSETCHAINS_NK[1]);
        }
        if (i == ASSETCHAINS_NUMALGOS)
        {
            LogPrintf("ASSETCHAINS_ALGO, %s not supported. using equihash\n", selectedAlgo.c_str());
        }

        ASSETCHAINS_LASTERA = GetArg("-ac_eras", 1);
        if ( ASSETCHAINS_LASTERA < 1 || ASSETCHAINS_LASTERA > ASSETCHAINS_MAX_ERAS )
        {
            ASSETCHAINS_LASTERA = 1;
            LogPrintf("ASSETCHAINS_LASTERA, if specified, must be between 1 and %u. ASSETCHAINS_LASTERA set to %" PRIu64 "\n", ASSETCHAINS_MAX_ERAS, ASSETCHAINS_LASTERA);
        }
        ASSETCHAINS_LASTERA -= 1;

        ASSETCHAINS_TIMELOCKGTE = (uint64_t)GetArg("-ac_timelockgte", _ASSETCHAINS_TIMELOCKOFF);
        ASSETCHAINS_TIMEUNLOCKFROM = GetArg("-ac_timeunlockfrom", 0);
        ASSETCHAINS_TIMEUNLOCKTO = GetArg("-ac_timeunlockto", 0);
        if ( ASSETCHAINS_TIMEUNLOCKFROM > ASSETCHAINS_TIMEUNLOCKTO )
        {
            LogPrintf("ASSETCHAINS_TIMELOCKGTE - must specify valid ac_timeunlockfrom and ac_timeunlockto\n");
            ASSETCHAINS_TIMELOCKGTE = _ASSETCHAINS_TIMELOCKOFF;
            ASSETCHAINS_TIMEUNLOCKFROM = ASSETCHAINS_TIMEUNLOCKTO = 0;
        }

        Split(GetArg("-ac_end",""), sizeof(ASSETCHAINS_ENDSUBSIDY)/sizeof(*ASSETCHAINS_ENDSUBSIDY),  ASSETCHAINS_ENDSUBSIDY, 0);
        Split(GetArg("-ac_reward",""), sizeof(ASSETCHAINS_REWARD)/sizeof(*ASSETCHAINS_REWARD),  ASSETCHAINS_REWARD, 0);
        Split(GetArg("-ac_halving",""), sizeof(ASSETCHAINS_HALVING)/sizeof(*ASSETCHAINS_HALVING),  ASSETCHAINS_HALVING, 0);
        Split(GetArg("-ac_decay",""), sizeof(ASSETCHAINS_DECAY)/sizeof(*ASSETCHAINS_DECAY),  ASSETCHAINS_DECAY, 0);
        Split(GetArg("-ac_notarypay",""), sizeof(ASSETCHAINS_NOTARY_PAY)/sizeof(*ASSETCHAINS_NOTARY_PAY),  ASSETCHAINS_NOTARY_PAY, 0);

        for ( int i = 0; i < ASSETCHAINS_MAX_ERAS; i++ )
        {
            if ( ASSETCHAINS_DECAY[i] == 100000000 && ASSETCHAINS_ENDSUBSIDY[i] == 0 )
            {
                ASSETCHAINS_DECAY[i] = 0;
                LogPrintf("ERA%u: ASSETCHAINS_DECAY of 100000000 means linear and that needs ASSETCHAINS_ENDSUBSIDY\n", i);
            }
            else if ( ASSETCHAINS_DECAY[i] > 100000000 )
            {
                ASSETCHAINS_DECAY[i] = 0;
                LogPrintf("ERA%u: ASSETCHAINS_DECAY cant be more than 100000000\n", i);
            }
        }

        MAX_BLOCK_SIGOPS = 60000;
        ASSETCHAINS_TXPOW = GetArg("-ac_txpow",0) & 3;
        ASSETCHAINS_FOUNDERS = GetArg("-ac_founders",0);// & 1;
		ASSETCHAINS_FOUNDERS_REWARD = GetArg("-ac_founders_reward",0);
        ASSETCHAINS_SUPPLY = GetArg("-ac_supply",10);
        if ( ASSETCHAINS_SUPPLY > (uint64_t)90*1000*1000000 )
        {
            LogPrintf("-ac_supply must be less than 90 billion\n");
            StartShutdown();
        }
        LogPrintf("ASSETCHAINS_SUPPLY %llu\n",(long long)ASSETCHAINS_SUPPLY);
        
        ASSETCHAINS_COMMISSION = GetArg("-ac_perc",0);
        memset(ASSETCHAINS_OVERRIDE_PUBKEY33, 0, sizeof(ASSETCHAINS_OVERRIDE_PUBKEY33));
        ASSETCHAINS_OVERRIDE_PUBKEY = GetArg("-ac_pubkey","");
        ASSETCHAINS_SCRIPTPUB = GetArg("-ac_script","");
        ASSETCHAINS_BEAMPORT = GetArg("-ac_beam",0);
        ASSETCHAINS_CODAPORT = GetArg("-ac_coda",0);
        ASSETCHAINS_CBMATURITY = GetArg("-ac_cbmaturity",0);
        ASSETCHAINS_ADAPTIVEPOW = GetArg("-ac_adaptivepow",0);
        std::string hexstr = GetArg("-ac_mineropret","");
        std::vector<uint8_t> Mineropret;
        if ( hexstr.size() != 0 )
        {
            Mineropret.resize(hexstr.size()/2);
            decode_hex(Mineropret.data(),hexstr.size()/2,(char *)hexstr.c_str());
            for (size_t i=0; i<Mineropret.size(); i++)
                LogPrintf("%02x",Mineropret[i]);
            LogPrintf(" Mineropret\n");
        }
        if ( ASSETCHAINS_COMMISSION != 0 && ASSETCHAINS_FOUNDERS_REWARD != 0 )
        {
            LogPrintf("cannot use founders reward and commission on the same chain.\n");
            StartShutdown();
        }
        if ( ASSETCHAINS_CC != 0 )
        {
            uint8_t prevCCi = 0;
            ASSETCHAINS_CCLIB = GetArg("-ac_cclib","");
            uint64_t ccenables[256];
            memset(ccenables, 0, sizeof(ccenables) );
            Split(GetArg("-ac_ccenable",""), sizeof(ccenables)/sizeof(*ccenables),  ccenables, 0);
            for (uint16_t i=0; i<256; i++)
            {
                if ( ccenables[i] != prevCCi && ccenables[i] != 0 )
                {
                    nonz++;
                    prevCCi = ccenables[i];
                    LogPrintf("%d ",(uint8_t)(ccenables[i] & 0xff));
                }
            }
            LogPrintf("nonz.%d ccenables[]\n",nonz);
            if ( nonz > 0 )
            {
                // disable all CCs
                for (uint16_t i=0; i<256; i++)
                {
                    ASSETCHAINS_CCDISABLES[i] = 1;
                    SETBIT(disablebits,i);
                }
                // enable chosen CCs
                for (uint16_t i=0; i<nonz; i++)
                {
                    CLEARBIT(disablebits,(ccenables[i] & 0xff));
                    ASSETCHAINS_CCDISABLES[ccenables[i] & 0xff] = 0;
                }
                CLEARBIT(disablebits,0); // always enable contract 0. Why is this here? -JMJ
            }
        }
        if ( ASSETCHAINS_BEAMPORT != 0 )
        {
            LogPrintf("can only have one of -ac_beam or -ac_coda\n");
            StartShutdown();
        }
        ASSETCHAINS_SELFIMPORT = GetArg("-ac_import",""); // BEAM, CODA, PUBKEY, GATEWAY
        if ( ASSETCHAINS_SELFIMPORT == "PUBKEY" )
        {
            if ( strlen(ASSETCHAINS_OVERRIDE_PUBKEY.c_str()) != 66 )
            {
                LogPrintf("invalid -ac_pubkey for -ac_import=PUBKEY\n");
                StartShutdown();
            }
        }
        else if ( ASSETCHAINS_SELFIMPORT == "BEAM" )
        {
            if (ASSETCHAINS_BEAMPORT == 0)
            {
                LogPrintf("missing -ac_beam for BEAM rpcport\n");
                StartShutdown();
            }
        }
        else if ( ASSETCHAINS_SELFIMPORT == "CODA" )
        {
            if (ASSETCHAINS_CODAPORT == 0)
            {
                LogPrintf("missing -ac_coda for CODA rpcport\n");
                StartShutdown();
            }
        }
        // else it can be gateway coin
        else if (!ASSETCHAINS_SELFIMPORT.empty() && (ASSETCHAINS_ENDSUBSIDY[0]!=1 || ASSETCHAINS_SUPPLY>0 || ASSETCHAINS_COMMISSION!=0))
        {
            LogPrintf("when using gateway import these must be set: -ac_end=1 -ac_supply=0 -ac_perc=0\n");
            StartShutdown();
        }
        

        if ( (ASSETCHAINS_STAKED= GetArg("-ac_staked",0)) > 100 )
            ASSETCHAINS_STAKED = 100;

        ASSETCHAINS_SAPLING = GetArg("-ac_sapling", -1);
        if (ASSETCHAINS_SAPLING == -1)
        {
            ASSETCHAINS_OVERWINTER = GetArg("-ac_overwinter", -1);
        }
        else
        {
            ASSETCHAINS_OVERWINTER = GetArg("-ac_overwinter", ASSETCHAINS_SAPLING);
        }
        if ( strlen(ASSETCHAINS_OVERRIDE_PUBKEY.c_str()) == 66 || ASSETCHAINS_SCRIPTPUB.size() > 1 )
        {
            if ( ASSETCHAINS_SUPPLY > 10000000000 )
            {
                LogPrintf("ac_pubkey or ac_script wont work with ac_supply over 10 billion\n");
                StartShutdown();
            }
            if ( ASSETCHAINS_NOTARY_PAY[0] != 0 )
            {
                LogPrintf("Assetchains NOTARY PAY cannot be used with ac_pubkey or ac_script.\n");
                StartShutdown();
            }
            if ( strlen(ASSETCHAINS_OVERRIDE_PUBKEY.c_str()) == 66 )
            {
                decode_hex(ASSETCHAINS_OVERRIDE_PUBKEY33,33,(char *)ASSETCHAINS_OVERRIDE_PUBKEY.c_str());
                calc_rmd160_sha256(ASSETCHAINS_OVERRIDE_PUBKEYHASH,ASSETCHAINS_OVERRIDE_PUBKEY33,33);
            }
            if ( ASSETCHAINS_COMMISSION == 0 && ASSETCHAINS_FOUNDERS != 0 )
            {
                if ( ASSETCHAINS_FOUNDERS_REWARD == 0 )
                {
                    ASSETCHAINS_COMMISSION = 53846154; // maps to 35%
                    LogPrintf("ASSETCHAINS_COMMISSION defaulted to 35%% when founders reward active\n");
                }
                else
                {
                    LogPrintf("ASSETCHAINS_FOUNDERS_REWARD set to %" PRIu64 "\n", ASSETCHAINS_FOUNDERS_REWARD);
                }
                /*else if ( ASSETCHAINS_SELFIMPORT.size() == 0 )
                {
                    //ASSETCHAINS_OVERRIDE_PUBKEY.clear();
                    LogPrintf("-ac_perc must be set with -ac_pubkey\n");
                }*/
            }
        }
        else
        {
            if ( ASSETCHAINS_COMMISSION != 0 )
            {
                ASSETCHAINS_COMMISSION = 0;
                LogPrintf("ASSETCHAINS_COMMISSION needs an ASSETCHAINS_OVERRIDE_PUBKEY and cant be more than 100000000 (100%%)\n");
            }
            if ( ASSETCHAINS_FOUNDERS != 0 )
            {
                ASSETCHAINS_FOUNDERS = 0;
                LogPrintf("ASSETCHAINS_FOUNDERS needs an ASSETCHAINS_OVERRIDE_PUBKEY or ASSETCHAINS_SCRIPTPUB\n");
            }
        }
        if ( ASSETCHAINS_ENDSUBSIDY[0] != 0 
                || ASSETCHAINS_REWARD[0] != 0 
                || ASSETCHAINS_HALVING[0] != 0 
                || ASSETCHAINS_DECAY[0] != 0 
                || ASSETCHAINS_COMMISSION != 0 
                || ASSETCHAINS_PUBLIC != 0 
                || ASSETCHAINS_PRIVATE != 0 
                || ASSETCHAINS_TXPOW != 0 
                || ASSETCHAINS_FOUNDERS != 0 
                || ASSETCHAINS_SCRIPTPUB.size() > 1 
                || ASSETCHAINS_SELFIMPORT.size() > 0 
                || ASSETCHAINS_OVERRIDE_PUBKEY33[0] != 0 
                || ASSETCHAINS_TIMELOCKGTE != _ASSETCHAINS_TIMELOCKOFF
                || ASSETCHAINS_ALGO != ASSETCHAINS_EQUIHASH 
                || ASSETCHAINS_LASTERA > 0 
                || ASSETCHAINS_BEAMPORT != 0 
                || ASSETCHAINS_CODAPORT != 0 
                || nonz > 0 
                || ASSETCHAINS_CCLIB.size() > 0 
                || ASSETCHAINS_FOUNDERS_REWARD != 0 
                || ASSETCHAINS_NOTARY_PAY[0] != 0 
                || ASSETCHAINS_BLOCKTIME != 60 
                || Mineropret.size() != 0 
                || (ASSETCHAINS_NK[0] != 0 && ASSETCHAINS_NK[1] != 0) 
                || KOMODO_SNAPSHOT_INTERVAL != 0 
                || ASSETCHAINS_EARLYTXIDCONTRACT != 0 
                || ASSETCHAINS_CBMATURITY != 0 
                || ASSETCHAINS_ADAPTIVEPOW != 0 )
        {
            LogPrintf("perc %.4f%% ac_pub=[%02x%02x%02x...] acsize.%d\n",dstr(ASSETCHAINS_COMMISSION)*100,ASSETCHAINS_OVERRIDE_PUBKEY33[0],ASSETCHAINS_OVERRIDE_PUBKEY33[1],ASSETCHAINS_OVERRIDE_PUBKEY33[2],(int32_t)ASSETCHAINS_SCRIPTPUB.size());
            extraptr = extrabuf;
            memcpy(extraptr,ASSETCHAINS_OVERRIDE_PUBKEY33,33), extralen = 33;

            // if we have one era, this should create the same data structure as it used to, same if we increase _MAX_ERAS
            for ( int i = 0; i <= ASSETCHAINS_LASTERA; i++ )
            {
                LogPrintf("ERA%u: end.%llu reward.%llu halving.%llu decay.%llu notarypay.%llu\n", i,
                       (long long)ASSETCHAINS_ENDSUBSIDY[i],
                       (long long)ASSETCHAINS_REWARD[i],
                       (long long)ASSETCHAINS_HALVING[i],
                       (long long)ASSETCHAINS_DECAY[i],
                       (long long)ASSETCHAINS_NOTARY_PAY[i]);

                // TODO: Verify that we don't overrun extrabuf here, which is a 256 byte buffer
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_ENDSUBSIDY[i]),(void *)&ASSETCHAINS_ENDSUBSIDY[i]);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_REWARD[i]),(void *)&ASSETCHAINS_REWARD[i]);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_HALVING[i]),(void *)&ASSETCHAINS_HALVING[i]);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_DECAY[i]),(void *)&ASSETCHAINS_DECAY[i]);
                if ( ASSETCHAINS_NOTARY_PAY[0] != 0 )
                    extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_NOTARY_PAY[i]),(void *)&ASSETCHAINS_NOTARY_PAY[i]);
            }

            if (ASSETCHAINS_LASTERA > 0)
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_LASTERA),(void *)&ASSETCHAINS_LASTERA);
            }

            // hash in lock above for time locked coinbase transactions above a certain reward value only if the lock above
            // param was specified, otherwise, for compatibility, do nothing
            if ( ASSETCHAINS_TIMELOCKGTE != _ASSETCHAINS_TIMELOCKOFF )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_TIMELOCKGTE),(void *)&ASSETCHAINS_TIMELOCKGTE);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_TIMEUNLOCKFROM),(void *)&ASSETCHAINS_TIMEUNLOCKFROM);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_TIMEUNLOCKTO),(void *)&ASSETCHAINS_TIMEUNLOCKTO);
            }

            if ( ASSETCHAINS_ALGO != ASSETCHAINS_EQUIHASH )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_ALGO),(void *)&ASSETCHAINS_ALGO);
            }

            uint64_t val = ASSETCHAINS_COMMISSION | (((int64_t)ASSETCHAINS_STAKED & 0xff) << 32) | (((uint64_t)ASSETCHAINS_CC & 0xffff) << 40) | ((ASSETCHAINS_PUBLIC != 0) << 7) | ((ASSETCHAINS_PRIVATE != 0) << 6) | ASSETCHAINS_TXPOW;
            extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(val),(void *)&val);
            
            if ( ASSETCHAINS_FOUNDERS != 0 )
            {
                uint8_t tmp = 1;
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(tmp),(void *)&tmp);
                if ( ASSETCHAINS_FOUNDERS > 1 )
                    extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_FOUNDERS),(void *)&ASSETCHAINS_FOUNDERS);
                if ( ASSETCHAINS_FOUNDERS_REWARD != 0 )
                {
                    LogPrintf( "set founders reward.%lld\n",(long long)ASSETCHAINS_FOUNDERS_REWARD);
                    extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_FOUNDERS_REWARD),(void *)&ASSETCHAINS_FOUNDERS_REWARD);
                }
            }
            if ( ASSETCHAINS_SCRIPTPUB.size() > 1 )
            {
                decode_hex(&extraptr[extralen],ASSETCHAINS_SCRIPTPUB.size()/2,(char *)ASSETCHAINS_SCRIPTPUB.c_str());
                extralen += ASSETCHAINS_SCRIPTPUB.size()/2;
                LogPrintf("append ac_script %s\n",ASSETCHAINS_SCRIPTPUB.c_str());
            }
            if ( ASSETCHAINS_SELFIMPORT.size() > 0 )
            {
                memcpy(&extraptr[extralen],(char *)ASSETCHAINS_SELFIMPORT.c_str(),ASSETCHAINS_SELFIMPORT.size());
                for (size_t i=0; i<ASSETCHAINS_SELFIMPORT.size(); i++)
                    LogPrintf("%c",extraptr[extralen+i]);
                LogPrintf(" selfimport\n");
                extralen += ASSETCHAINS_SELFIMPORT.size();
            }
            if ( ASSETCHAINS_BEAMPORT != 0 )
                extraptr[extralen++] = 'b';
            if ( ASSETCHAINS_CODAPORT != 0 )
                extraptr[extralen++] = 'c';
            LogPrintf("extralen.%d before disable bits\n",extralen);
            if ( nonz > 0 )
            {
                memcpy(&extraptr[extralen],disablebits,sizeof(disablebits));
                extralen += sizeof(disablebits);
            }
            if ( ASSETCHAINS_CCLIB.size() > 1 )
            {
                for (size_t i=0; i<ASSETCHAINS_CCLIB.size(); i++)
                {
                    extraptr[extralen++] = ASSETCHAINS_CCLIB[i];
                    LogPrintf("%c",ASSETCHAINS_CCLIB[i]);
                }
                LogPrintf(" <- CCLIB name\n");
            }
            if ( ASSETCHAINS_BLOCKTIME != 60 )
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_BLOCKTIME),(void *)&ASSETCHAINS_BLOCKTIME);
            if ( Mineropret.size() != 0 )
            {
                for (size_t i=0; i<Mineropret.size(); i++)
                    extraptr[extralen++] = Mineropret[i];
            }
            if ( ASSETCHAINS_NK[0] != 0 && ASSETCHAINS_NK[1] != 0 )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_NK[0]),(void *)&ASSETCHAINS_NK[0]);
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_NK[1]),(void *)&ASSETCHAINS_NK[1]);
            }
            if ( KOMODO_SNAPSHOT_INTERVAL != 0 )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(KOMODO_SNAPSHOT_INTERVAL),(void *)&KOMODO_SNAPSHOT_INTERVAL);
            }
            if ( ASSETCHAINS_EARLYTXIDCONTRACT != 0 )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_EARLYTXIDCONTRACT),(void *)&ASSETCHAINS_EARLYTXIDCONTRACT);
            }
            if ( ASSETCHAINS_CBMATURITY != 0 )
            {
                extralen += iguana_rwnum(1,&extraptr[extralen],sizeof(ASSETCHAINS_CBMATURITY),(void *)&ASSETCHAINS_CBMATURITY);
            }
            if ( ASSETCHAINS_ADAPTIVEPOW != 0 )
                extraptr[extralen++] = ASSETCHAINS_ADAPTIVEPOW;
        }
        
        //addn = GetArg("-seednode","");
        //if ( strlen(addn.c_str()) > 0 )
        //    ASSETCHAINS_SEED = 1;

        MAX_MONEY = komodo_max_money();
        int32_t baseid;
        if ( (baseid = komodo_baseid(chainName.symbol().c_str())) >= 0 && baseid < 32 )
        {
            LogPrintf("baseid.%d MAX_MONEY.%s %.8f\n",baseid,chainName.symbol().c_str(),(double)MAX_MONEY/SATOSHIDEN);
        }

        if ( ASSETCHAINS_CC >= KOMODO_FIRSTFUNGIBLEID && MAX_MONEY < 1000000LL*SATOSHIDEN )
            MAX_MONEY = 1000000LL*SATOSHIDEN;
        if ( KOMODO_BIT63SET(MAX_MONEY) != 0 )
            MAX_MONEY = KOMODO_MAXNVALUE;
        LogPrintf("MAX_MONEY %llu %.8f\n",(long long)MAX_MONEY,(double)MAX_MONEY/SATOSHIDEN);
        uint16_t tmpport = komodo_port(chainName.symbol().c_str(),ASSETCHAINS_SUPPLY,&ASSETCHAINS_MAGIC,extraptr,extralen);
        if ( GetArg("-port",0) != 0 )
        {
            ASSETCHAINS_P2PPORT = GetArg("-port",0);
            LogPrintf("set p2pport.%u\n",ASSETCHAINS_P2PPORT);
        } 
        else 
            ASSETCHAINS_P2PPORT = tmpport;

        char* dirname = nullptr;
        while ( (dirname= (char *)GetDataDir(false).string().c_str()) == 0 || dirname[0] == 0 )
        {
            LogPrintf("waiting for datadir (%s)\n",dirname);
#ifndef _WIN32
            sleep(3);
#else
            boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
#endif
        }
        if ( !chainName.isKMD() )
        {
            if ( chainName.isSymbol("KMD") )
            {
                LogPrintf("can't have assetchain named KMD\n");
                StartShutdown();
            }
            uint16_t port;
            if ( (port= komodo_userpass(ASSETCHAINS_USERPASS,chainName.symbol().c_str())) != 0 )
                ASSETCHAINS_RPCPORT = port;
            else komodo_configfile(chainName.symbol().c_str(),ASSETCHAINS_P2PPORT + 1);

            if (ASSETCHAINS_CBMATURITY != 0)
                Params().SetCoinbaseMaturity(ASSETCHAINS_CBMATURITY);
            else if (ASSETCHAINS_LASTERA == 0 || is_STAKED(chainName.symbol()) != 0)
                Params().SetCoinbaseMaturity(1);
            if (Params().CoinbaseMaturity() < 1)
            {
                LogPrintf("ac_cbmaturity must be >0, shutting down\n");
                StartShutdown();
            }
        }
        if ( ASSETCHAINS_RPCPORT == 0 )
            ASSETCHAINS_RPCPORT = ASSETCHAINS_P2PPORT + 1;
        uint8_t magic[4];
        iguana_rwnum(1,magic,sizeof(ASSETCHAINS_MAGIC),(void *)&ASSETCHAINS_MAGIC);
        char magicstr[9];
        for (i=0; i<4; i++)
            sprintf(&magicstr[i<<1],"%02x",magic[i]);
        magicstr[8] = 0;
#ifndef FROM_CLI
        char fname[512];
        sprintf(fname,"%s_7776",chainName.symbol().c_str());
        if ( (fp= fopen(fname,"wb")) != 0 )
        {
            int8_t notarypay = 0;
            if ( ASSETCHAINS_NOTARY_PAY[0] != 0 )
                notarypay = 1;
            char *iguanafmtstr = (char *)"curl --url \"http://127.0.0.1:7776\" --data \"{\\\"conf\\\":\\\"%s.conf\\\",\\\"path\\\":\\\"${HOME#\"/\"}/.komodo/%s\\\",\\\"unitval\\\":\\\"20\\\",\\\"zcash\\\":1,\\\"RELAY\\\":-1,\\\"VALIDATE\\\":0,\\\"prefetchlag\\\":-1,\\\"poll\\\":100,\\\"active\\\":1,\\\"agent\\\":\\\"iguana\\\",\\\"method\\\":\\\"addcoin\\\",\\\"startpend\\\":4,\\\"endpend\\\":4,\\\"services\\\":129,\\\"maxpeers\\\":8,\\\"newcoin\\\":\\\"%s\\\",\\\"name\\\":\\\"%s\\\",\\\"hasheaders\\\":1,\\\"useaddmultisig\\\":0,\\\"netmagic\\\":\\\"%s\\\",\\\"p2p\\\":%u,\\\"rpc\\\":%u,\\\"pubval\\\":60,\\\"p2shval\\\":85,\\\"wifval\\\":188,\\\"txfee_satoshis\\\":\\\"10000\\\",\\\"isPoS\\\":0,\\\"minoutput\\\":10000,\\\"minconfirms\\\":2,\\\"genesishash\\\":\\\"027e3758c3a65b12aa1046462b486d0a63bfa1beae327897f56c5cfb7daaae71\\\",\\\"protover\\\":170002,\\\"genesisblock\\\":\\\"0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a000000000000000000000000000000000000000000000000000000000000000029ab5f490f0f0f200b00000000000000000000000000000000000000000000000000000000000000fd4005000d5ba7cda5d473947263bf194285317179d2b0d307119c2e7cc4bd8ac456f0774bd52b0cd9249be9d40718b6397a4c7bbd8f2b3272fed2823cd2af4bd1632200ba4bf796727d6347b225f670f292343274cc35099466f5fb5f0cd1c105121b28213d15db2ed7bdba490b4cedc69742a57b7c25af24485e523aadbb77a0144fc76f79ef73bd8530d42b9f3b9bed1c135ad1fe152923fafe98f95f76f1615e64c4abb1137f4c31b218ba2782bc15534788dda2cc08a0ee2987c8b27ff41bd4e31cd5fb5643dfe862c9a02ca9f90c8c51a6671d681d04ad47e4b53b1518d4befafefe8cadfb912f3d03051b1efbf1dfe37b56e93a741d8dfd80d576ca250bee55fab1311fc7b3255977558cdda6f7d6f875306e43a14413facdaed2f46093e0ef1e8f8a963e1632dcbeebd8e49fd16b57d49b08f9762de89157c65233f60c8e38a1f503a48c555f8ec45dedecd574a37601323c27be597b956343107f8bd80f3a925afaf30811df83c402116bb9c1e5231c70fff899a7c82f73c902ba54da53cc459b7bf1113db65cc8f6914d3618560ea69abd13658fa7b6af92d374d6eca9529f8bd565166e4fcbf2a8dfb3c9b69539d4d2ee2e9321b85b331925df195915f2757637c2805e1d4131e1ad9ef9bc1bb1c732d8dba4738716d351ab30c996c8657bab39567ee3b29c6d054b711495c0d52e1cd5d8e55b4f0f0325b97369280755b46a02afd54be4ddd9f77c22272b8bbb17ff5118fedbae2564524e797bd28b5f74f7079d532ccc059807989f94d267f47e724b3f1ecfe00ec9e6541c961080d8891251b84b4480bc292f6a180bea089fef5bbda56e1e41390d7c0e85ba0ef530f7177413481a226465a36ef6afe1e2bca69d2078712b3912bba1a99b1fbff0d355d6ffe726d2bb6fbc103c4ac5756e5bee6e47e17424ebcbf1b63d8cb90ce2e40198b4f4198689daea254307e52a25562f4c1455340f0ffeb10f9d8e914775e37d0edca019fb1b9c6ef81255ed86bc51c5391e0591480f66e2d88c5f4fd7277697968656a9b113ab97f874fdd5f2465e5559533e01ba13ef4a8f7a21d02c30c8ded68e8c54603ab9c8084ef6d9eb4e92c75b078539e2ae786ebab6dab73a09e0aa9ac575bcefb29e930ae656e58bcb513f7e3c17e079dce4f05b5dbc18c2a872b22509740ebe6a3903e00ad1abc55076441862643f93606e3dc35e8d9f2caef3ee6be14d513b2e062b21d0061de3bd56881713a1a5c17f5ace05e1ec09da53f99442df175a49bd154aa96e4949decd52fed79ccf7ccbce32941419c314e374e4a396ac553e17b5340336a1a25c22f9e42a243ba5404450b650acfc826a6e432971ace776e15719515e1634ceb9a4a35061b668c74998d3dfb5827f6238ec015377e6f9c94f38108768cf6e5c8b132e0303fb5a200368f845ad9d46343035a6ff94031df8d8309415bb3f6cd5ede9c135fdabcc030599858d803c0f85be7661c88984d88faa3d26fb0e9aac0056a53f1b5d0baed713c853c4a2726869a0a124a8a5bbc0fc0ef80c8ae4cb53636aa02503b86a1eb9836fcc259823e2692d921d88e1ffc1e6cb2bde43939ceb3f32a611686f539f8f7c9f0bf00381f743607d40960f06d347d1cd8ac8a51969c25e37150efdf7aa4c2037a2fd0516fb444525ab157a0ed0a7412b2fa69b217fe397263153782c0f64351fbdf2678fa0dc8569912dcd8e3ccad38f34f23bbbce14c6a26ac24911b308b82c7e43062d180baeac4ba7153858365c72c63dcf5f6a5b08070b730adb017aeae925b7d0439979e2679f45ed2f25a7edcfd2fb77a8794630285ccb0a071f5cce410b46dbf9750b0354aae8b65574501cc69efb5b6a43444074fee116641bb29da56c2b4a7f456991fc92b2\\\",\\\"debug\\\":0,\\\"seedipaddr\\\":\\\"%s\\\",\\\"sapling\\\":1,\\\"notarypay\\\":%i}\"";
            fprintf(fp,iguanafmtstr,name.c_str(),name.c_str(),name.c_str(),name.c_str(),magicstr,ASSETCHAINS_P2PPORT,ASSETCHAINS_RPCPORT,"78.47.196.146",notarypay);
            fclose(fp);
        } else LogPrintf("error creating (%s)\n",fname);
#endif
        if ( ASSETCHAINS_CC < 2 )
        {
            if ( KOMODO_CCACTIVATE != 0 )
            {
                ASSETCHAINS_CC = 2;
                LogPrintf("smart utxo CC contracts will activate at height.%d\n",KOMODO_CCACTIVATE);
            }
            else if ( ccEnablesHeight[0] != 0 )
            {
                ASSETCHAINS_CC = 2;
                LogPrintf("smart utxo CC contract %d will activate at height.%d\n",(int32_t)ccEnablesHeight[0],(int32_t)ccEnablesHeight[1]);
            }
        }
    }
    else
    {
        // -ac_name not passed, we are on the KMD chain
        set_kmd_user_password_port(ntz_dest_path);
    }
    int32_t dpowconfs = KOMODO_DPOWCONFS;
    if ( !chainName.isKMD() )
    {
        BITCOIND_RPCPORT = GetArg("-rpcport", ASSETCHAINS_RPCPORT);
        //LogPrintf("(%s) port.%u chain params initialized\n",chainName.symbol().c_str(),BITCOIND_RPCPORT);
        if ( chainName.isSymbol("PIRATE") && ASSETCHAINS_HALVING[0] == 77777 )
        {
            ASSETCHAINS_HALVING[0] *= 5;
            LogPrintf("PIRATE halving changed to %d %.1f days ASSETCHAINS_LASTERA.%llu\n",(int32_t)ASSETCHAINS_HALVING[0],(double)ASSETCHAINS_HALVING[0]/1440,(long long)ASSETCHAINS_LASTERA);
        }
        else if ( ASSETCHAINS_PRIVATE != 0 )
        {
            LogPrintf("-ac_private for a non-PIRATE chain is not supported. The only reason to have an -ac_private chain is for total privacy and that is best achieved with the largest anon set. PIRATE has that and it is recommended to just use PIRATE\n");
            StartShutdown();
        }
        // Set cc enables for all existing ac_cc chains here. 
        if ( chainName.isSymbol("AXO") )
        {
            // No CCs used on this chain yet.
            CCDISABLEALL;
        }
        if ( chainName.isSymbol("CCL") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
            CCENABLE(EVAL_TOKENS);
            CCENABLE(EVAL_HEIR);
        }
        if ( chainName.isSymbol("COQUI") )
        {
            CCDISABLEALL;
            CCENABLE(EVAL_DICE);
            CCENABLE(EVAL_CHANNELS);
            CCENABLE(EVAL_ORACLES);
            CCENABLE(EVAL_ASSETS);
            CCENABLE(EVAL_TOKENS);
        }
        if ( chainName.isSymbol("DION") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
        }
        
        if ( chainName.isSymbol("EQL") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
        }
        if ( chainName.isSymbol("ILN") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
        }
        if ( chainName.isSymbol("OUR") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
        }
        if ( chainName.isSymbol("ZEXO") )
        {
            // No CCs used on this chain yet. 
            CCDISABLEALL;
        }
        if ( chainName.isSymbol("SEC") )
        {
            CCDISABLEALL;
            CCENABLE(EVAL_ASSETS);
            CCENABLE(EVAL_TOKENS);
            CCENABLE(EVAL_ORACLES);
        }
        if ( chainName.isSymbol("KMDICE") )
        {
            CCDISABLEALL;
            CCENABLE(EVAL_FAUCET);
            CCENABLE(EVAL_DICE);
            CCENABLE(EVAL_ORACLES);
        }
    } 
    else 
        BITCOIND_RPCPORT = GetArg("-rpcport", BaseParams().RPCPort());
    KOMODO_DPOWCONFS = GetArg("-dpowconfs",dpowconfs);
    if ( chainName.isKMD() 
            || chainName.isSymbol("SUPERNET") 
            || chainName.isSymbol("DEX") 
            || chainName.isSymbol("COQUI") 
            || chainName.isSymbol("PIRATE") 
            || chainName.isSymbol("KMDICE") )
        KOMODO_EXTRASATOSHI = 1;
}
