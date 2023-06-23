#pragma once
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <simdjson.h>

enum class version_type {
    release,
    snapshot
};


struct ManifestVersionInfo {
    std::string id;
    std::string type;
    std::string url;
    std::string release_time;
    std::string sha1sum;
};

struct ShortVersionInfo {
    std::string id;
    bool is_release;
};


struct ManifestInfo {
    std::string latest_snapshot;
    std::string latest_release;

    std::vector<ShortVersionInfo> version_names;
    std::unordered_map<std::string, ManifestVersionInfo> version_map;
};




ManifestInfo process_manifest_json(std::string_view str){
    ManifestInfo info;
    simdjson::padded_string paddedResponse(str);
    simdjson::ondemand::parser parser;
    auto document = parser.iterate(paddedResponse);

    auto latest_doc = document["latest"];
    info.latest_release = latest_doc["release"].get_string().value();
    info.latest_snapshot = latest_doc["snapshot"].get_string().value();

    for (auto version : document["versions"]){
        ManifestVersionInfo versionInfo;
        versionInfo.id = version["id"].get_string().value();
        versionInfo.type = version["type"].get_string().value();
        versionInfo.url = version["url"].get_string().value();
        versionInfo.release_time = version["releaseTime"].get_string().value();
        versionInfo.sha1sum = version["sha1"].get_string().value();

        info.version_names.emplace_back(versionInfo.id, versionInfo.type == "release");
        info.version_map[versionInfo.id] = std::move(versionInfo);
    }
    return info;
}


struct VersionUrlResult {
    std::string url;
    std::string sha1;
};

VersionUrlResult process_version_json(std::string_view str) {
    simdjson::padded_string padded_response(str);
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(padded_response);
    auto server_dl = doc["downloads"]["server"];
    VersionUrlResult result;
    result.url = server_dl["url"].get_string().value();
    result.sha1 = server_dl["sha1"].get_string().value();

    return result;
}