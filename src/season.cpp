//
// Created by moss on 9/29/22.
//

#include "season.h"

namespace dropout_dl {
	episode season::get_episode(const std::string& html_data, int& start_point, const cookie& session_cookie) {
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
						std::string episode_text = get_substring_in(html_data, R"(<span class='media-identifier media-episode'>)", "</span>", i);
						int episode_number = -1;
						if (!episode_text.empty()) {
							episode_number = get_int_in_string(episode_text);
						}
						return episode(html_data.substr(i, j), session_cookie, this->series_name, this->name, episode_number, this->season_number, false, this->download_captions);
					}
				}
			}
		}
		std::cerr << "SEASON PARSE ERROR: Error finding episode" << std::endl;
		exit(8);
	}

	std::vector<episode> season::get_episodes(const cookie& session_cookie) {
		std::vector<episode> out;

		std::string site_video(R"(class="browse-item-link" data-track-event="site_video")");

		for (int i = 0; i < this->page_data.size(); i++) {
			if (substr_is(this->page_data, i, site_video)) {
				episode e = get_episode(this->page_data, i, session_cookie);
				if (e.episode_url.empty()) {
					continue;
				}
				std::cout << '\t' << e.name << '\n';
				out.push_back(e);
			}
		}

		return out;
	}

	int season::get_season_number(const std::string& url) {
		std::string reversed_number;
		for (int i = url.length() - 1; i >= 0 && url[i] != ':'; i--) {
			if (isdigit(url[i])) {
				reversed_number += url[i];
			}
		}
		std::string number;
		for (int i = reversed_number.length() - 1, j = 0; i >= 0; i--, j++) {
			number[j] += reversed_number[i];
		}

		return std::stoi(number);
	}

	void season::download(const std::string &quality, const std::string &series_directory) {
		if (!std::filesystem::is_directory(series_directory)) {
			std::filesystem::create_directories(series_directory);
			std::cout << "Creating series directory" << '\n';
		}

		std::string dir = series_directory + "/" + this->name;

		for (auto& ep : episodes) {
			ep.download(quality, dir);
		}
	}
} // dropout_dl
