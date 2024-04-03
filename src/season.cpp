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
						return episode(html_data.substr(i, j), session_cookie, this->series_name, this->name, episode_number, this->season_number, false, this->download_captions, this->download_captions_only);
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(rate_limit));
		}
		std::cerr << "SEASON PARSE ERROR: Error finding episode" << std::endl;
		exit(8);
	}


	void season::add_episodes_to_vector(const cookie& session_cookie, const std::string& page_data, std::vector<episode>& episodes) {
		const std::string site_video(R"(class="browse-item-link" data-track-event="site_video")");
		for (int i = 0; i < page_data.size(); i++) {
			if (substr_is(page_data, i, site_video)) {
				episode e = get_episode(page_data, i, session_cookie);
				if (e.episode_url.empty()) {
					continue;
				}
				std::cout << '\t' << e.name << '\n';
				episodes.push_back(e);
			}
		}
	}

	std::vector<episode> season::get_episodes(const cookie& session_cookie) {
		std::vector<episode> out;

		add_episodes_to_vector(session_cookie, this->page_data, out);


		// Find episodes hidden behind "Show More". This is sort of a hack but it should be fine.
		// What we do is get the page data for the next page (this is what the "show more" button does behind the scenes) and check if it exists.
		// If it does get the episodes and check the next page, otherwise just return from what we got from the first page
		long status_code = -1;
		int page_index = 2;
		while (true) {
			std::string next_page_url = this->url + "?page=" + std::to_string(page_index);

			std::string next_page_data = get_generic_page(next_page_url, &status_code);

			if (status_code != 200) {
				break;
			}

			add_episodes_to_vector(session_cookie, next_page_data, out);

			page_index++;
		}

		return out;
	}

	int season::get_season_number(const std::string& url) {
		std::string reversed_number = "";
		for (int i = url.length() - 1; i >= 0 && url[i] != ':'; i--) {
			if (isdigit(url[i])) {
				reversed_number += url[i];
			}
		}
		std::string number = "";
		for (int i = reversed_number.length() - 1; i >= 0; i--) {
			number += reversed_number[i];
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
