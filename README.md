# RandoRater

CLI tool to rate the difficulty of a Hollow Knight item randomizer seed (with the end goal of true ending) according to the required skips for progression. Difficulties are assigned for all logical loadouts of a check in `parsed.xml`. Probably only compatible up to and including randomized soul totems. Run as `main.exe [flags]`.

Loadouts with a difficulty of -1 are considered uninitialized and will cause an error unless the flag `--ignore-bad-difficulty` is passed in the CLI arguments, in which case it will be treated as a difficulty of 0.

Depends on [pugixml](https://github.com/zeux/pugixml). Compile `main.cpp` for the rating executable, which must be run from the command line. Compile `logicparser.xml` to update `parsed.xml` from the relevant logic files.