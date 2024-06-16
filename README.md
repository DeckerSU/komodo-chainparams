`komodo-chainparams` is a small utility designed to determine the P2P and RPC ports of Komodo assetchains without launching a Komodo daemon (`komodod`). It can be useful in scripts, GitHub Workflows, etc. The utility simply copies the original port number determination code from the daemon and is highly unoptimized. In other words, it is just a "cut off" version of the original `komodod` code, modified to launch separately.

It has been successfully tested with g++ 11.4.0 and 14.0.0 under Ubuntu 22.04.4 LTS. However, on other *nix systems, there may be compilation issues due to headers, etc. The build system does not include a configure file to handle compatibility headers. Instead, you can choose preprocessor directives directly in the Makefile. Additionally, a Docker image based on Alpine Linux is provided.

In the future, it would be better to rewrite it in Python, as the algorithm is simply the concatenation of byte representations of parameters and hashing the resulting string (`extrabuf`). However, for now, this working example is sufficient.

#### Build

```bash
make -f ./Makefile all -j$(nproc)
```

#### Launch

```bash
./komodo-chainparams <chain parameters>
```
Example:
```bash
./komodo-chainparams -ac_name=DOC -ac_supply=90000000000 -ac_reward=100000000 -ac_cc=3 -ac_staked=10 -addnode=65.21.77.109 -addnode=65.21.51.47 -addnode=209.222.101.247 -addnode=103.195.100.32 | jq .
```

#### Output

```json
{
  "chainname": "DOC",
  "magic": 1450148915,
  "magic_bytes": "33846f56",
  "magic_hex": "566f8433",
  "p2pport": 62415,
  "rpcport": 62416
}
```

### Docker

Build:

```bash
docker build --no-cache -t komodo-chainparams .
```

Launch:

```bash
docker run --rm komodo-chainparams <chain_parameters> | jq .
```
Example:
```bash
docker run --rm komodo-chainparams -ac_name=DOC -ac_supply=90000000000 -ac_reward=100000000 -ac_cc=3 -ac_staked=10 -addnode=65.21.77.109 -addnode=65.21.51.47 -addnode=209.222.101.247 -addnode=103.195.100.32 | jq .
```

Run from DockerHub:

```
docker pull deckersu/komodo-chainparams:latest
docker run --rm deckersu/komodo-chainparams:latest <chain_parameters> | jq .
```
