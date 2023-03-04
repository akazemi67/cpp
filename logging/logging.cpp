#include "logging.h"
#include <yaml-cpp/yaml.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <iostream>
#include <memory>

std::tuple<bool, bool, std::string> parseConfig(){
    bool console = false;
    bool file = false;
    std::string path;
    try {
        YAML::Node config = YAML::LoadFile("/opt/P2pChat/logging.yml");
        auto logging = config["logging"];

        for (const auto& item : logging) {
            if (item.IsMap()) {
                for (YAML::const_iterator it = item.begin(); it != item.end(); ++it) {
                    auto name = it->first.as<std::string>();
                    if("console"==name)
                        console = true;
                    else if("file"==name)
                        file = true;
                    else if("path"==name)
                        path = it->second.as<std::string>();
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    std::cout << "Console:" << console << "   file:" << file <<"   path:" << path << std::endl;
    return {console, file, path};
}

void init_logging(){
    auto [console, file, path] = parseConfig();
    std::vector<spdlog::sink_ptr> sinks;
    if(console)
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if(file)
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.c_str(), 1024 * 1024, 5));//filename, maxsize, maxfiles
    logger = std::make_shared<spdlog::logger>( "P2pChat", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(10));
    spdlog::info("Successfully initialized logger.");
}
