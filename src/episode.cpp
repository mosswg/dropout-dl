//
// Created by moss on 9/28/22.
//
#include "episode.h"
#include <regex>

namespace dropout_dl {
	nlohmann::json episode::get_meta_data_json(const std::string& html_data) {
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
						// -1 and +2 to include opening and closing brackets that are normally excluded.
						auto json_string = html_data.substr(i-1, j+2);
							// std::cout << json_string << std::endl;
						return nlohmann::json::parse(json_string);
					}
				}
			}
		}
		return "ERROR";
	}

	std::string episode::get_series_name(const nlohmann::json& meta_data) {
		if (meta_data.find("PROPERTIES") != meta_data.end() &&
			meta_data["PROPERTIES"].find("CANONICAL_COLLECTION") != meta_data["PROPERTIES"].end() &&
			meta_data["PROPERTIES"]["CANONICAL_COLLECTION"].find("parent") != meta_data["PROPERTIES"]["CANONICAL_COLLECTION"].end() &&
			meta_data["PROPERTIES"]["CANONICAL_COLLECTION"]["parent"].find("name") != meta_data["PROPERTIES"]["CANONICAL_COLLECTION"]["parent"].end()) {
			return meta_data["PROPERTIES"]["CANONICAL_COLLECTION"]["parent"]["name"];
		}
		return "";
	}

	int episode::get_episode_number(const std::string& page_data, int season_number) {
		std::string open_string = "Season " + std::to_string(season_number) + ", Episode ";
		std::string close_string = "\n";

		std::string episode_num_str = get_substring_in(page_data, open_string, close_string, 0);

		int episode_number = -1;
		if (!episode_num_str.empty()) {
			episode_number = get_int_in_string(episode_num_str);
		}

		return episode_number;
	}

	std::string episode::get_season_name(const nlohmann::json& meta_data) {
		if (meta_data.find("PROPERTIES") != meta_data.end() &&
			meta_data["PROPERTIES"].find("CANONICAL_TITLE") != meta_data["PROPERTIES"].end()) {
			return meta_data["PROPERTIES"]["COLLECTION_TITLE"];
		}
		return "";
	}

	std::string episode::get_episode_name(const nlohmann::json& meta_data) {
		if (meta_data.find("PROPERTIES") != meta_data.end() &&
			meta_data["PROPERTIES"].find("VIDEO_TITLE") != meta_data["PROPERTIES"].end()) {
			return meta_data["PROPERTIES"]["VIDEO_TITLE"];
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
		// If we already have them, use those
		if (!video_qualities.empty() && !audio_qualities.empty()) {
			return video_qualities;
		}
		auto& streams = this->config_json["request"]["files"]["progressive"];
		if(!streams.empty()) { // Older format, not from a CDN I guess
			for (const auto& stream : streams) {
				this->video_qualities.push_back(stream["quality"]);
				this->video_quality_segments.push_back({(std::string)stream["url"]});
				if (this->verbose) {
					std::cout << "Found quality: " << video_qualities.back() << std::endl;
				}
			}
			return video_qualities;
		}
		
		// Newer format, with a CDN
		if (this->verbose) {
			std::cout << "Getting from cdn\n";
		}
		this->is_from_cdn = true;

		std::string default_cdn = this->config_json["request"]["files"]["dash"]["default_cdn"];
		std::string cdn_url = this->config_json["request"]["files"]["dash"]["cdns"][default_cdn]["url"];

		if (this->verbose) {
			std::cout << "cdn: " << cdn_url << "\n";
		}

		// Process the JSON data at this CDN's url and deduce the base_url
		auto cdn_json_str = get_generic_page(cdn_url);
		nlohmann::json cdn_json;
		try {
			cdn_json = nlohmann::json::parse(cdn_json_str);
		}
		catch (nlohmann::detail::parse_error e) {
			std::cerr << "EPISODE ERROR: Couldn't parse CDN json: " << cdn_json_str << "\n";
			return video_qualities;
		}
		
		//TODO: These regexes might be hyperspecific to how the akfire_interconnect_quic CDN works.
		//      Make sure that the other ones also follow this base URL format.
		std::string base_url, url_suffix;
		if(std::smatch base_url_match; std::regex_search(cdn_url,base_url_match,std::regex("^https.+v2/"))) {
			base_url = base_url_match[0].str();
			std::string cdn_json_base_url = cdn_json["base_url"];
			// cdn_json_base_url is normally in the format "../../foo/bar/". The following extracts the "/foo/bar/"
			if(std::regex_search(cdn_json_base_url,base_url_match,std::regex("(?:/\\w+)+/?$")))
				url_suffix = base_url_match[0].str();
		}
		if(base_url.empty() || url_suffix.empty()) {
			std::cerr << RED << "ERROR: Unable to build base URL for CDN access\n" << RESET;
			return video_qualities;
		}
		base_url += url_suffix;

		// Extract the video & audio info and record the segment URLs for later download
		for (const auto& video : cdn_json["video"]) {
			this->video_qualities.push_back(std::to_string((long)video["height"]) + "p");
			std::string video_base_url = base_url + (std::string)video["base_url"];
			std::vector<std::string> video_segment_urls = {};
			for (const auto& seg : video["segments"]) {
				video_segment_urls.push_back(video_base_url + (std::string)seg["url"]);
			}

			this->video_initial_segment_quality.push_back((std::string)video["init_segment"]);
			this->video_quality_segments.push_back(video_segment_urls);
			if (this->verbose) {
				std::cout << "Found video quality: " << video_qualities.back() << std::endl;
			}
		}
		for (const auto& audio : cdn_json["audio"]) {
			std::string audio_base_url = base_url + (std::string)audio["base_url"];
			this->audio_qualities.push_back(std::to_string((long)audio["bitrate"]));
			std::vector<std::string> audio_segment_urls = {};
			for (const auto& seg : audio["segments"]) {
				audio_segment_urls.push_back(audio_base_url + (std::string)seg["url"]);
			}
			this->audio_initial_segment_quality.push_back((std::string)audio["init_segment"]);
			this->audio_quality_segments.push_back(audio_segment_urls);
			if (this->verbose) {
				std::cout << "Found Audio quality: " << audio_qualities.back() << std::endl;
			}
		}

		return video_qualities;
	}

	std::string episode::get_captions_url() {
		/*
		  Different episodes are going to have different start and end points in this config file.
		  These are the options that contributors have found so far.
		  Add to these vectors when new possibilities are discovered.
		*/

		/// NOTE: If dropout adds other language subtitles we want to support those but currently english is the only language they have (as far as im aware)

		auto& text_tracks = this->config_json["request"]["text_tracks"];

		for (const auto& text_track : text_tracks) {
			if (text_track["kind"] == "captions" || text_track["kind"] == "subtitles") {
				if (text_track["lang"] == "en" || text_track["lang"] == "en-US") {
					if (this->verbose) {
						std::cout << "captions url: " << text_track["url"] << '\n';
					}
					return text_track["url"];
				}
			}
		}

		std::cerr << RED << "ERROR: Could not find captions for episode \"" << this->name << RESET << '\n';

		return "";
	}


	int episode::get_video_quality_index(const std::string& quality) {
		for (int i = 0; i < video_qualities.size(); i++) {
			if (video_qualities[i] == quality) {
				return i;
			}
		}
		std::cerr << "ERROR: quality of " << quality << " not found\nPossible qualities: ";
		for (int i = 0; i < video_qualities.size(); i++) {
			std::cerr << video_qualities[i];
			if (i != video_qualities.size() - 1) {
				std::cerr << ", ";
			}
		}
		exit(6);
	}

	std::string episode::get_video_segment_url(int quality, int segment) {
		return video_quality_segments[quality][segment];
	}

	bool episode::get_video_segment_data(int quality, int segment_index, std::string& curl_buffer, const std::string& filename) {
		CURL* curl = curl_easy_init();
		if(!curl)
			return false;
		std::string progress_name = filename + " - video segment: " + std::to_string(segment_index);
		CURLcode res;
		curl_buffer.clear(); // C++ standard claims this is O(n) but all implementations are constant-time so we're chill
		curl_easy_setopt(curl, CURLOPT_URL, get_video_segment_url(quality, segment_index).c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropout_dl::WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_buffer);
		// curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		// curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, dropout_dl::curl_progress_func);
		// curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress_name6);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(curl_buffer.empty())
			std::cerr << YELLOW << "WARN: Video segment was an empty packet!\n" << RESET;
		if(this->verbose)
			std::cout << "Packet size: " << curl_buffer.size() << '\n';
		return true;
	}

	std::string episode::get_audio_segment_url(int quality, int segment) {
		return audio_quality_segments[quality][segment];
	}

	std::string episode::get_audio_segment_data(int quality, int segment_index, const std::string& filename) {
		CURL* curl = curl_easy_init();
		std::string progress_name = filename + " - audio segment: " + std::to_string(segment_index);
		CURLcode res;
		if(curl) {
			std::string out;
			out.reserve(2048);
			curl_easy_setopt(curl, CURLOPT_URL, this->get_audio_segment_url(quality, segment_index).c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropout_dl::WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
			// curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
			// curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, dropout_dl::curl_progress_func);
			// curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress_name);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			if(out.empty())
				std::cerr << YELLOW << "WARN: Audio segment was an empty packet!\n" << RESET;
			return out;
		}
		return "CURL ERROR";
	}


	void episode::download_quality(const std::string& quality, const std::string& base_directory, const std::string& filename, bool lowest_audio_quality) {
		if (!std::filesystem::is_directory(base_directory)) {
			std::filesystem::create_directories(base_directory);
			if (this->verbose) {
				std::cout << "Creating quality directory" << '\n';
			}
		}

		/// TODO: make this verify lowest quality and possibly add options for any specific quality
		int audio_quality_index = 0;
		if (lowest_audio_quality) {
			audio_quality_index = this->audio_qualities.size() - 1;
		}

		std::string filepath = base_directory + "/" + filename;

		if(std::filesystem::is_regular_file(filepath)) {
			std::cout << YELLOW << "File already exists: " << filepath << RESET << '\n';
			return;
		}
		if (!check_existing(quality,filepath + ".mp4") && !this->download_captions_only) {
			int video_quality_index = get_video_quality_index(quality);
			std::string tmp;
			if (this->verbose) {
				std::cout << "Number of Video Segments: " << this->video_quality_segments[video_quality_index].size() << "\n";
			}
			std::fstream out(filepath + ".m4s",
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

			out << dropout_dl::base64_decode(video_initial_segment_quality[video_quality_index]);

			int number_of_video_segs = this->video_quality_segments[video_quality_index].size();

			std::string curl_buffer; // Using a buffer string at the top here to minimize memory reallocation
			curl_buffer.reserve(1 << 20); // A megabyte
			for (int i = 0; i < number_of_video_segs; i++) {
				dropout_dl::segment_progress_func(filepath + ".m4s", i, number_of_video_segs);
				if(!this->get_video_segment_data(video_quality_index, i, curl_buffer, filepath)) {
					std::cerr << RED << "ERROR: Unknown error occurred in Curl during video segment download.\n" << RESET;
					return; 
				}
				// Writing the segment directly to the out fstream would've been the simplest thing to do,
				// but we do actually want to use a buffer so that we can double-check that the output is valid here.
				if(curl_buffer == "{\"status\":403,\"title\":\"Forbidden\",\"detail\":\"403 Forbidden\"}\n" ||
				   (curl_buffer.size() < 1024 && curl_buffer.find("403")) // Trying to avoid searching for a 403 when it's a megabyte packet
				) {
					std::cerr << RED << "ERROR: Unable to get video segment #" << i << " due to 403 response from CDN\n" << RESET;
					return;
				}
				out << curl_buffer;
			}
			out.close();
			std::cout << std::endl;

			if (this->verbose) {
				std::cout << "Number of Audio Segments: " << this->audio_quality_segments.front().size() << "\n";
			}
			out = std::fstream(filepath + ".m4a",
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

			out << dropout_dl::base64_decode(audio_initial_segment_quality[audio_quality_index]);

			for (int i = 0; i < this->audio_quality_segments[audio_quality_index].size(); i++) {
				dropout_dl::segment_progress_func(filepath + ".m4a", i, number_of_video_segs);
				tmp = this->get_audio_segment_data(audio_quality_index, i, filepath);
				if (tmp == "Not Found") {
					std::cerr << YELLOW << "Could not get audio segment " << i << RESET << "\n";
					break;
				}
				out << tmp;
			}
			out.close();

			std::cout << std::endl;

		}

		if (!this->captions_url.empty()) {
			std::fstream captions_file(filepath + ".vtt",
							 std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

			captions_file << get_generic_page(this->captions_url);

		}

		#ifdef DROPOUT_DL_FFMPEG
		std::string ffmpeg_cmd = "ffmpeg -i '" + filepath + ".m4a' -i '" + filepath + ".m4s'";
		if (!this->captions_url.empty()) {
			ffmpeg_cmd += " -i '" + filepath + ".vtt' -metadata:s:s:0 language=eng";
		}
		ffmpeg_cmd += " -c copy '" + filepath + ".mp4'";
		if (!this->verbose) {
			ffmpeg_cmd += " > /dev/null";
		}
		else {
			std::cout << ffmpeg_cmd << "\n";
		}
		int ffmpeg_ret = std::system(std::string(ffmpeg_cmd).c_str());

		if (ffmpeg_ret == 0) {
			std::filesystem::remove(filepath + ".m4a");
			std::filesystem::remove(filepath + ".m4s");
		}
		#endif

		std::cout << GREEN << filepath << RESET;

		std::cout << '\n';
	}


	void episode::download(const std::string& quality, const std::string& series_directory, std::string filename) {
		if (filename.empty()) {
			std::string prefix;
			if (this->series != "") {
				prefix = this->series + " - ";
			}
			if (this->episode_number != 0) {
				if (this->episode_number == -1) {
					/// Episode file without season or episode number in case of special episode
					filename = prefix + this->name;
				}
				else if (this->season_number != 0) {
					filename = prefix + "S" + ((this->season_number < 10) ? "0" : "") + std::to_string(this->season_number) + "E" + ((this->episode_number < 10) ? "0" : "") + std::to_string(this->episode_number) + " - " + this->name;
				}
				else {
					filename = prefix + this->season + " Episode " + std::to_string(this->episode_number) + " - " + this->name;
				}
			}
			else {
				filename = prefix + this->season + " - " + this->name;
			}
			filename = format_filename(filename);
		}


		if (quality == "all") {
			for (const auto &possible_video_quality: this->video_qualities) {
				this->download_quality(possible_video_quality, series_directory + possible_video_quality, filename);
			}
		}
		else if (quality == "highest") {
			std::string highest_quality;
			int highest_value = 0;
			int current_value;
			for (const auto &possible_quality: this->video_qualities) {
				current_value = get_int_in_string(possible_quality);
				if (current_value > highest_value) {
					highest_value = current_value;
					highest_quality = possible_quality;
				}
			}
			this->download_quality(highest_quality, series_directory, filename);
		}
		else if (quality == "lowest") {
			std::string lowest_quality;
			int lowest_value = INT_MAX;
			int current_value;
			for (const auto &possible_quality: this->video_qualities) {
				current_value = get_int_in_string(possible_quality);
				if (current_value < lowest_value) {
					lowest_value = current_value;
					lowest_quality = possible_quality;
				}
			}
			this->download_quality(lowest_quality, series_directory, filename, true);
		}
		else {
			this->download_quality(quality, series_directory, filename);
		}
	}

	/// TODO: Reimplement size checking. I.E. replace an existing file if the size is not the same
	bool episode::check_existing(const std::string &quality, const std::string& filename){
		std::filesystem::path file_path = filename + ".mp4";
		if (std::filesystem::exists(file_path)) {
			return true;
		}
		else return false;
	}
} // dropout_dl
