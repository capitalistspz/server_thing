#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <optional>
#include "manifest_info.hpp"

std::string const manifest_url = "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json";

std::filesystem::path default_path = std::filesystem::current_path() / "versions";
std::optional<std::filesystem::path> custom_path;
std::string chosen_version;

inline std::filesystem::path output_file_path(std::string_view version_name){
    if (custom_path) {
        return custom_path.value();
    }
    else {
        auto path = default_path / version_name;
        if (!std::filesystem::exists(path))
            std::filesystem::create_directories(path);
        return path / "server.jar";
    }
}


int main(int argc, char** argv) {
    if (argc > 1){
        for (auto arg_index = 1; arg_index < argc; ++arg_index){
            std::string_view arg = argv[arg_index];
            if (arg == "--path"){
                arg_index += 1;
                if (arg_index < argc){
                    custom_path = argv[arg_index];
                }
                else {
                    std::cerr << "You must specify a path\n";
                    return 1;
                }
            }
            else if (arg == "--version"){
                arg_index += 1;
                if (arg_index < argc){
                    chosen_version = argv[arg_index];
                }
                else {
                    std::cerr << "You must specify a version\n";
                    return 2;
                }

            }
            else if (arg == "--help"){
                std::cout << "server_thing [OPTIONS?]\n"
                             "--help                        This message\n"
                             "--version [GAME_VERSION]      Select a version to download\n"
                             "--path [SERVER JAR PATH]      The path that the jar should be downloaded to.\n";
                return 0;
            }
        }
    }
    std::cout << "Getting version manifest.\n";
    cpr::Response response = cpr::Get(cpr::Url(manifest_url));
    if (response.error){
        std::cerr << "Failed to get the version manifest: " << response.error.message;
        return 3;
    }
    std::cout << "Finished getting version manifest.\n";

    auto manifestInfo = process_manifest_json(response.text);

    std::string version_name;
    std::string version_json_url;
    if (!chosen_version.empty()){
        version_name = chosen_version;

        auto info = manifestInfo.version_map.find(version_name);
        if (info == manifestInfo.version_map.end()){
            std::cerr << "Version '" << version_name << "' not found. Exiting. ";
            return 5;
        }
        version_json_url = info->second.url;
    }
    else {
        std::cout << "[1] Latest release (" << manifestInfo.latest_release << ")\n[2] Latest snapshot (" << manifestInfo.latest_snapshot << ")\n[3] Show releases\n[4] Show releases with snapshots\n";

        unsigned int selection = 0;
        while (selection == 0 || selection > 4){
            std::cin >> selection;
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            if (selection == 1){
                version_json_url = manifestInfo.version_map[manifestInfo.latest_release].url;
                version_name = manifestInfo.latest_release;
            }
            else if (selection == 2){
                version_json_url = manifestInfo.version_map[manifestInfo.latest_snapshot].url;
                version_name = manifestInfo.latest_snapshot;
            }
            else if (selection <= 4){
                for (const auto& release_ver : manifestInfo.version_names){
                    if (selection == 3 && !release_ver.is_release)
                        continue;
                    std::cout << release_ver.id << '\n';
                }
                auto info = manifestInfo.version_map.cend();
                do {
                    std::cout << "\nEnter a version name:\n";
                    std::getline(std::cin, version_name);

                    info = manifestInfo.version_map.find(version_name);
                }
                while (info == manifestInfo.version_map.cend());
                version_json_url = info->second.url;

            }

        }
    }
    std::cout << "Getting info for version '" << version_name << "' from " << version_json_url << "\n";
    const auto versionJsonResponse = cpr::Get(cpr::Url(version_json_url));
    VersionUrlResult versionResult;
    try {
        versionResult = process_version_json(versionJsonResponse.text);
    }
    catch (simdjson::simdjson_error const& e){
        std::cerr << "Failed to process version json: " << e.what() << "\n(it is possible that this failed because server jars for versions prior to 1.2.5 are not available from Mojang)";
        return 6;
    }

    std::cout << "Finished getting version info.\n";
    auto output_path = output_file_path(version_name);

    std::cout << "Downloading server.jar for version '" << version_name << "' from " << versionResult.url << " to " << output_path << "\n";

    std::ofstream output_file(output_path);
    auto download_response = cpr::Download(output_file, cpr::Url(versionResult.url));
    if (download_response.error){
        std::cerr << "Failed to download server jar: " << download_response.error.message;
        return 4;
    }
    std::cout << "Finished downloading server jar.\n";
    return 0;
}
