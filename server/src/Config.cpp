#include "Config.h"
#include <map>
#include <sstream>
#include <fstream>


std::shared_ptr<Config> Config::instance__;

void Config::initGlobalConfig(const std::string &filename) {
    instance__.reset(new Config(filename));
}


template<class T>
static void readInto(const std::string &name, const std::map<std::string, std::string> &src, T &dest) {
    if (src.find(name) != src.end()) {
        std::istringstream in(src.at(name));
        in >> dest;
    }
}

Config::Config(const std::string &filename) {
    std::ifstream in(filename);
    std::string line;
    std::map<std::string, std::string> pairs;
    std::string k, v;
    while (std::getline(in, line)) {
        std::istringstream sin(line);
        sin >> k >> v;
        pairs[k] = v;
    }

    std::string db_host, db_user, db_password, db_database;

#define READ_OPTION(NAME) do{readInto(#NAME, pairs, NAME);}while(0);
    READ_OPTION(host)
    READ_OPTION(port)
    READ_OPTION(max_clients)
    READ_OPTION(num_workers)
    READ_OPTION(debug)
    READ_OPTION(db_host)
    READ_OPTION(db_user)
    READ_OPTION(db_password)
    READ_OPTION(db_database)
    READ_OPTION(logfile)
#undef READ_OPTION

    db_config.host = db_host;
    db_config.user = db_user;
    db_config.password = db_password;
    db_config.database = db_database;

}