#ifndef P2PCHAT_DATAIO_H
#define P2PCHAT_DATAIO_H

#include "PeersInfo.h"
#include <vector>
#include <memory>

#define DEFAULT_CONFIG_FILE_PATH "/opt/P2pChat/data_sources.json"

class PeersDataReader {
public:
    virtual std::unique_ptr<std::vector<Peer>> readData() = 0;
    virtual ~PeersDataReader() = default;
};

class DataReaderFactory {
private:
    std::string configFilePath;
    std::unique_ptr<PeersDataReader> createDataReader(const std::string& type, const std::string& path);
public:
    explicit DataReaderFactory(const std::string &configFilePath);

    std::vector<std::unique_ptr<PeersDataReader>> createDataReadersFromConfig();
};

#endif
