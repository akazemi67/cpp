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
        auto result = std::make_unique<std::vector<Peer>>();
        std::ifstream dataFile(jsonFilePath);
        json peersArray = json::parse(dataFile);

        for(auto &peerInfo : peersArray){
            try {
                Peer p(peerInfo["name"], peerInfo["ip"], peerInfo["port"]);
                getLogger()->info("Peer data fetched: {}, {}, {}", p.name, p.IPv4, p.port);
                result->push_back(std::move(p));
            } catch (json::exception &e){
                getLogger()->warn("Cannot parse json element: {} {}", e.id, e.what());
            }
        }
        return result;
    }
};

class CsvDataReader : public PeersDataReader {
private:
    std::string csvFilePath;
public:
    explicit CsvDataReader(const std::string &csvFilePath) : csvFilePath(csvFilePath) {}

    std::unique_ptr<std::vector<Peer>> readData() override {
        auto result = std::make_unique<std::vector<Peer>>();
        std::ifstream file(csvFilePath);

        const char delimiter = ',';
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string column;
            std::vector<std::string> row;

            while (std::getline(ss, column, delimiter)) {
                row.push_back(column);
            }
            if(row.size()<3)
                continue;
            Peer p(row[0], row[1], std::stoi(row[2]));
            getLogger()->info("Peer data fetched: {}, {}, {}", p.name, p.IPv4, p.port);
            result->push_back(std::move(p));
        }
        return result;
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
            getLogger()->info("Datasource {} with path {} exists.", name, pathStr);
            std::unique_ptr<PeersDataReader> dataReader = createDataReader(name, pathStr);
            if (dataReader != nullptr)
                readers.push_back(std::move(dataReader));

    }
    return readers;
}

