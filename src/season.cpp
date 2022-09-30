//
// Created by moss on 9/29/22.
//

#include "season.h"

namespace dropout_dl {
    episode get_episode(const std::string& html_data, int& start_point, const std::vector<cookie>& cookies) {
        int link_start = 0;
        for (int i = start_point; i > 0; i--) {
            if (substr_is(html_data, i, "<a")) {
                link_start = i;
                break;
            }
            else if (substr_is(html_data, i, "<")) {
                // Invalid episode place. Return empty value.
                return {};
            }
        }

        for (int i = link_start; i < html_data.size(); i++) {
            if (substr_is(html_data, i, "href=\"")) {
                i += 6;
                for (int j = 0; j + i < html_data.size(); j++) {
                    if (html_data[i + j] == '"') {
                        start_point += 15;
                        return {html_data.substr(i, j), cookies};
                    }
                }
            }
        }
        std::cerr << "SEASON PARSE ERROR: Error finding episode" << std::endl;
        exit(8);
    }

    std::vector<episode> season::get_episodes(const std::string &html_data, const std::vector<cookie>& cookies) {
        std::vector<episode> out;

        std::string site_video(R"(class="browse-item-link" data-track-event="site_video")");


        for (int i = 0; i < html_data.size(); i++) {
            if (substr_is(html_data, i, site_video)) {
                episode e = get_episode(html_data, i, cookies);
                if (e.episode_url.empty()) {
                    continue;
                }
                std::cout << e.episode_number << ": " << e.name << ": " << e.episode_url << '\n';
                out.push_back(e);
            }
        }

        return out;
    }

    void season::download(const std::string &quality, const std::string &series_directory) {
        if (!std::filesystem::is_directory(series_directory)) {
            std::filesystem::create_directories(series_directory);
            std::cout << "Creating series directory" << '\n';
        }

        std::string dir = series_directory + "/" + this->name;

        std::replace(dir.begin(), dir.end(), ' ', '_');

        std::replace(dir.begin(), dir.end(), ',', '_');

        for (auto& ep : episodes) {
            ep.download(quality, dir);
        }
    }
} // dropout_dl