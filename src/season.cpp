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

	std::vector<episode> season::get_episodes(const std::vector<cookie>& cookies) {
		std::vector<episode> out;

		std::string site_video(R"(class="browse-item-link" data-track-event="site_video")");

		int number_of_episodes = 0;
		for (int i = 0; i < this->page_data.size(); i++) {
			if (substr_is(this->page_data, i, site_video)) {
				episode e = get_episode(this->page_data, i, cookies);
				if (e.episode_url.empty()) {
					continue;
				}
				e.episode_number = ++number_of_episodes;
				e.season_number = this->season_number;
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
