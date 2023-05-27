//
// Created by moss on 9/28/22.
//
#include "episode.h"


namespace dropout_dl {
	std::string episode::get_meta_data_json(const std::string& html_data) {
		std::string data_start("window.Page = {");
		char data_open = '{';
		char data_close = '}';
		char current_char;
		// The current grouping depth. 1 because we only use it after we're inside the data brackets
		int grouping_depth = 1;
		for (int i = 0; i < html_data.size(); i++) {
			if (substr_is(html_data, i, data_start)) {
				i += data_start.size();
				for (int j = 0; j + i < html_data.size(); j++) {
					current_char = html_data[j + i];
					if (current_char == data_open) {
						grouping_depth++;
					}
					else if(current_char == data_close) {
						grouping_depth--;
					}

					if (grouping_depth == 0) {
						return(html_data.substr(i, j));
					}
				}
			}
		}
		return "ERROR";
	}

	// episode statics
	std::string episode::get_series_name(const std::string& meta_data) {
		int title_start = -1;
		std::string parent_title("\"parent\"");
		std::string series_title_title("\"name\"");
		for (int i = 0; i < meta_data.size(); i++) {
			if (substr_is(meta_data, i, parent_title)) {
				// Skip "VIDEO_TITLE", the following colon, and the opening quotation mark.
				i += parent_title.size() + 2;


				int j;
				for (j = 0; meta_data[i + j] != '}' && i + j < meta_data.size(); j++);

				std::string series_data = meta_data.substr(i, j);

				for (j = 0; j < series_data.size(); j++) {
					if (substr_is(series_data, j, series_title_title)) {
						// Skip "name", the following colon, and the opening quotation mark.
						j += series_title_title.size() + 2;

						int k;
						for (k = 0; j + k < series_data.size() && series_data[j + k] != '"'; k++);

						return format_name_string(series_data.substr(j, k));
					}
				}
			}
		}
		return "ERROR";
	}

	std::string episode::get_season_name(const std::string& meta_data) {
		std::string season_title_title("\"COLLECTION_TITLE\"");
		for (int i = 0; i < meta_data.size(); i++) {
			if (substr_is(meta_data, i, season_title_title)) {
				// Skip "VIDEO_TITLE", the following colon, and the opening quotation mark.
				i += season_title_title.size() + 2;


				int j;
				for (j = 0; meta_data[i + j] != '"' && i + j < meta_data.size(); j++);

				return format_name_string(meta_data.substr(i, j));
			}
		}
		return "ERROR";
	}

	std::string episode::get_episode_name(const std::string& meta_data) {
		std::string video_title_title("\"VIDEO_TITLE\"");
		for (int i = 0; i < meta_data.size(); i++) {
			if (substr_is(meta_data, i, video_title_title)) {
				// Skip "VIDEO_TITLE", the following colon, and the opening quotation mark.
				i += video_title_title.size() + 2;

				int j;
				for (j = 0; meta_data[i + j] != '"' && i + j < meta_data.size(); j++) {
					// skip checking for quotes if prefaced by a forward slash
					if (meta_data[i + j] == '\\') {
						j++;
					}
				}

				return format_name_string(meta_data.substr(i, j));
			}
		}
		return "ERROR";
	}

	std::string episode::get_embed_url(const std::string& html_data) {
		std::string config("window.VHX.config");
		std::string embed_url("embed_url: ");
		for (int i = 0; i < html_data.size(); i++) {
			if (substr_is(html_data, i, config)) {
				for (int j = i + config.size(); j < html_data.size(); j++) {
					if (substr_is(html_data, j, embed_url)) {
						for (int k = 0; k < html_data.size(); k++) {
							if (html_data[k + j + embed_url.size() + 1] == '"') {
								return html_data.substr(j + embed_url.size() + 1, k);
							}
						}
					}
				}
			}
		}
		return "";
	}

	std::string episode::get_config_url(const std::string& html_data) {
		std::string OTTdata("OTTData");
		std::string config_url("\"config_url\"");
		int remaining_quotes = 1;
		int url_start = -1;
		for (int i = 0; i < html_data.size(); i++) {
			if (substr_is(html_data, i, OTTdata)) {
				for (int j = i + OTTdata.size(); j < html_data.size(); j++) {
					if (substr_is(html_data, j, config_url)) {
						for (int k = 0; k < html_data.size() - (i + OTTdata.size()); k++) {
							char c = html_data[j + k + config_url.size()];
							if (remaining_quotes != 0) {
								if (html_data[j + k + config_url.size()] == '"') {
									remaining_quotes--;
								}
								continue;
							}
							else if (url_start == -1) {
								url_start = j + k + config_url.size();
							}

							if (html_data[url_start + k] == '"') {
								return html_data.substr(url_start, k);
							}
						}
					}
				}
			}
		}
		return "";
	}

	std::string episode::get_episode_page(const std::string& url, const std::string& session_cookie, bool verbose) {
		CURLcode ret;
		CURL *hnd;
		struct curl_slist *slist1;

		std::string episode_data;

		slist1 = nullptr;
		slist1 = curl_slist_append(slist1, "Accept: text/html");
		slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en");
		slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
		slist1 = curl_slist_append(slist1, "DNT: 1");
		slist1 = curl_slist_append(slist1, "Connection: keep-alive");
		slist1 = curl_slist_append(slist1, ("Cookie: locale_det=en; _session=" + session_cookie + ";").c_str());
		slist1 = curl_slist_append(slist1, "Upgrade-Insecure-Requests: 1");
		slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: document");
		slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: navigate");
		slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
		slist1 = curl_slist_append(slist1, "Sec-GPC: 1");

		hnd = curl_easy_init();
		curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
		curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
		curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
		curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(hnd, CURLOPT_VERBOSE, verbose);

		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &episode_data);

		std::string header;
		curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, WriteCallback);
		curl_easy_setopt(hnd, CURLOPT_HEADERDATA, &header);

		ret = curl_easy_perform(hnd);


		curl_easy_cleanup(hnd);
		hnd = nullptr;
		curl_slist_free_all(slist1);
		slist1 = nullptr;

		return episode_data;
	}

	std::vector<std::string> episode::get_qualities() {
		if (!qualities.empty()) {
			return qualities;
		}
		int i = 0;
		bool video_section = false;

		const std::string quality_marker = R"("quality":")";
		for (; i < config_data.size(); i++ ) {
			// std::cout << i << "/" << javascript_data.size() << ": " << javascript_data[i] << ": " << javascript_data.substr(i, 17) << ": " << video_section << "\n";
			if (config_data.substr(i, 9) == "video/mp4") {
				video_section = true;
			}

			if (video_section && config_data.substr(i, quality_marker.size()) == quality_marker) {
				i += quality_marker.size();
				for (int j = 0; j + i < config_data.size(); j++) {
					if (config_data[i + j] == '"') {
						this->qualities.push_back(config_data.substr(i, j));
						if (this->verbose) {
							std::cout << "Found quality (" << i << " + " << j << "): " << qualities.back() << std::endl;
						}
						break;
					}
				}
				for (int j = i; j > 0; j--) {
					// std::cout << i << ": " << javascript_data[i] << ": " << javascript_data.substr(i-7, 7) << "\n";
					if (this->config_data.substr(j-7, 7) == R"("url":")") {
						for (int k = 0; k < i - j; k++) {
							if (config_data[j + k] == '"') {
								this->quality_urls.emplace_back(config_data.substr(j, k));
								if (this->verbose) {
									std::cout << "Found url (" << j << " + " << k << "): " << quality_urls.back()
											  << std::endl;
								}
								break;
							}
						}
						break;
					}
				}
			}
		}
		return qualities;
	}

	std::string episode::get_captions_url() {
		std::string start = "\"lang\":\"en\",\"url\":\"";
		std::string end = "\",\"kind\":\"captions\"";

		if (this->config_data.find(end) == std::string::npos) {
			return "";
		}

		std::string captions_url = dropout_dl::get_substring_in(this->config_data, start, end);
		if (this->verbose) {
			std::cout << "captions url: " << captions_url << "\n";
		}

		return captions_url;
	}

	std::string episode::get_video_url(const std::string& quality) {
		for (int i = 0; i < qualities.size(); i++) {
			if (qualities[i] == quality) {
				return quality_urls[i];
			}
		}
		std::cerr << "ERROR: quality of " << quality << " not found\nPossible qualities: ";
		for (int i = 0; i < qualities.size(); i++) {
			std::cerr << qualities[i];
			if (i != qualities.size() - 1) {
				std::cerr << ", ";
			}
		}
		exit(6);
	}

	std::string episode::get_video_data(const std::string &quality, const std::string& filename) {
		CURL* curl = curl_easy_init();
		CURLcode res;
		if(curl) {
			std::string out;

			curl_easy_setopt(curl, CURLOPT_URL, get_video_url(quality).c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropout_dl::WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, dropout_dl::curl_progress_func);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &filename);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			return out;
		}
		return "CURL ERROR";
	}


	void episode::download_quality(const std::string& quality, const std::string& base_directory, const std::string& filename) {
		if (!std::filesystem::is_directory(base_directory)) {
			std::filesystem::create_directories(base_directory);
			if (this->verbose) {
				std::cout << "Creating quality directory" << '\n';
			}
		}

		std::string filepath = base_directory + "/" + filename;

		if(std::filesystem::is_regular_file(filepath)) {
			std::cout << YELLOW << "File already exists: " << filepath << RESET << '\n';
			return;
		}
		if (!checkExisting(quality,filepath)){
			std::fstream out(filepath + ".mp4",
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

			out << this->get_video_data(quality, filepath) << std::endl;
		}

		if (!this->captions_url.empty()) {
			std::fstream captions_file(filepath + ".vtt",
							 std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

			captions_file << get_generic_page(this->captions_url);

		}

		std::cout << GREEN << filepath << RESET;

		std::cout << '\n';
	}


	void episode::download(const std::string& quality, const std::string& series_directory, std::string filename) {
		if (filename.empty()) {
			if (this->episode_number != 0) {
				if (this->episode_number == -1) {
					/// Episode file without season or episode number in case of special episode
					filename = this->series + " - " + this->name;
				}
				else if (this->season_number != 0) {
					filename = this->series + " - S" + ((this->season_number < 10) ? "0" : "") + std::to_string(this->season_number) + "E" + ((this->episode_number < 10) ? "0" : "") + std::to_string(this->episode_number) + " - " + this->name;
				}
				else {
					filename = this->series + " - " + this->season + " Episode " + std::to_string(this->episode_number) + " - " + this->name;
				}
			}
			else {
				filename = this->series + " - " + this->season + " - " + this->name;
			}
			filename = format_filename(filename);
		}


		if (quality == "all") {
			for (const auto &possible_quality: this->qualities) {
				this->download_quality(possible_quality, series_directory + possible_quality, filename);
			}
		} else {
			this->download_quality(quality, series_directory, filename);
		}
	}

	bool episode::checkExisting(const std::string &quality, const std::string& filename){
		std::filesystem::path filePath = filename + ".mp4";
		double fileSize;
		CURL* curl = curl_easy_init();
    	CURLcode res;
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, get_video_url(quality).c_str());
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // Set to HTTP HEAD request
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropout_dl::EmptyWriteCallback);

			res = curl_easy_perform(curl);

			if (res == CURLE_OK) {
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileSize);
			}
			curl_easy_cleanup(curl);
    	}
		if (std::filesystem::exists(filePath)) {
        	std::uintmax_t fileSizeDisk = std::filesystem::file_size(filePath);
				if (fileSizeDisk-1 == fileSize){
					return true;
				}
				else return false;
		}
		else return false;
	}
} // dropout_dl
