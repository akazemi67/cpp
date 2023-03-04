#include <fstream>
#include "dataio.h"
#include "json.hpp"
#include "logging.h"

using json = nlohmann::json;

class JsonDataReader : public PeersDataReader{
private:
    std::string jsonFilePath;
public:
    explicit JsonDataReader(const std::string &jsonFilePath) : jsonFilePath(jsonFilePath) {}

    std::unique_ptr<std::vector<Peer>> readData() override {
        return {};
    }
};

class CsvDataReader : public PeersDataReader {
private:
    std::string csvFilePath;
public:
    explicit CsvDataReader(const std::string &csvFilePath) : csvFilePath(csvFilePath) {}

    std::unique_ptr<std::vector<Peer>> readData() override {
        return {};
    }
};

class SqliteDataReader : public PeersDataReader{
private:
    std::string dbFilePath;
public:
    explicit SqliteDataReader(const std::string &dbFilePath) : dbFilePath(dbFilePath) {}

    std::unique_ptr<std::vector<Peer>> readData() override {
        return {};
    }
};

DataReaderFactory::DataReaderFactory(const std::string &configFilePath) : configFilePath(configFilePath) {}

std::unique_ptr<PeersDataReader> DataReaderFactory::createDataReader(const std::string& type,
                                                                     const std::string& path) {
    if(type == "json")
        return std::make_unique<JsonDataReader>(path);
    if(type == "csv")
        return std::make_unique<CsvDataReader>(path);
    if(type == "sqlite")
        return std::make_unique<SqliteDataReader>(path);
    return nullptr;
}

std::vector<std::unique_ptr<PeersDataReader>> DataReaderFactory::createDataReadersFromConfig() {
    std::vector<std::unique_ptr<PeersDataReader>> readers;
    std::ifstream configFile(configFilePath);
    json config = json::parse(configFile);

    for(auto& [name, path] : config["data_sources"].items()){
            std::string pathStr = config["data_sources"][name];
            logger->info("Datasource {} with path {} exists.", name, pathStr);
            std::unique_ptr<PeersDataReader> dataReader = createDataReader(name, pathStr);
            if (dataReader != nullptr)
                readers.push_back(std::move(dataReader));

    }
    return readers;
}

