#include "util.h"

namespace dropout_dl {
	// dropout-dl helpers
	bool substr_is(const std::string& string, int start, const std::string& test_str) {
		if (test_str.size() != test_str.size())
			return false;

		for (int i = start, j = 0; i < start + test_str.size(); i++, j++) {
			if (string[i] != test_str[j]) {
				return false;
			}
		}
		return true;
	}

	void replace_all(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
	}


	std::string remove_leading_and_following_whitespace(const std::string& str) {
		int start = 0;
		int end = str.length() - 1;

		for (; str[start] == ' ' || str[start] == '\t' || str[start] == '\n'; start++);
		for (; str[end] == ' ' || str[end] == '\t' || str[end] == '\n'; end--);

		return str.substr(start, end - start + 1);
	}

	std::string replace_html_character_codes(const std::string& str) {
		std::string out;

		for (int i = 0; i < str.size(); i++) {
			if (substr_is(str, i, "&#")) {
				i += 2;
				char code = 0;

				if (i > str.size() - 4) {
					if (str[str.size() - 1] == ';') {
						// Numerical character code length is two at the end of the string

						code = str[str.size() - 2] - '0';
						code += (str[str.size() - 3] - '0') * 10;
						i += 2;
					}
				}
				else {
					if (str[i + 3] == ';') {
						// Numerical character code length is three
						code = str[i + 2] - '0';
						code += (str[i + 1] - '0') * 10;
						code += (str[i] - '0') * 10;
						i += 3;
					}
					else if (str[i + 2] == ';'){
						code = str[i + 1] - '0';
						code += (str[i] - '0') * 10;
						i += 2;
					}
					else {
						std::cerr << "HTML CHAR CODE ERROR: Code with numerical length of one used\n";
						exit(11);
					}
				}

				if (code < 32) {
					std::cerr << "HTML CHAR CODE ERROR: Control Character Decoded. This is not supported and likely an error.\n";
					exit(11);
				}

				out += code;
			}
			else {
				out += str[i];
			}
		}

		return out;
	}

	std::string remove_escaped_characters(const std::string& str) {
		std::string out;

		for (int i = 0; i < str.size(); i++) {
			if (str[i] == '\\') {
				i++;
			}
			out += str[i];
		}

		return out;
	}


	std::string format_name_string(const std::string& str) {
		return remove_escaped_characters(replace_html_character_codes(remove_leading_and_following_whitespace(str)));
	}

	std::string format_filename(const std::string& str) {
		std::string out;

		for (int i = 0; i < str.size(); i++) {
			char c = str[i];

			// Skip these
			if (c == '?' || c == ':' || c == '\\') {
				continue;
			}
			// Replace these with dashes
			else if (c == '/') {
				out += '-';
			}
			else {
				out += c;
			}
		}

		return out;
	}

#if defined(__WIN32__)
		#include <windows.h>
		msec_t time_ms(void)
		{
			return timeGetTime();
		}
	#else
	#include <sys/time.h>
		long time_ms()
		{
			timeval tv{};
			gettimeofday(&tv, nullptr);
			return tv.tv_sec * 1000 + tv.tv_usec / 1000;
		}
	#endif


	long current_time;
	long last_progress_timestamp;

	int curl_progress_func(void* filename, curl_off_t total_to_download, curl_off_t downloaded, curl_off_t total_to_upload, curl_off_t uploaded) {
		const double number_chars = 50;
		const char* full_character = "▓";
		const char* empty_character = "░";

		current_time = time_ms();
		if (current_time - 50 > last_progress_timestamp) {
			// Percent of the file downloaded. Adding one to round up so that when its done it shows as a full bar rather than missing one.
			double percent_done = (((double)downloaded / (double)total_to_download) * number_chars) + 1;
			double percent_done_clone = percent_done;
			std::cout << *(std::string*)filename << " [";
			while (percent_done_clone-- > 0) {
				std::cout << full_character;
			}
			while (percent_done++ < number_chars) {
				std::cout << empty_character;
			}
			std::cout << "] " << downloaded / 1048576 << "MiB / " << total_to_download / 1048576 << "MiB                           ";
			putchar('\r');
			last_progress_timestamp = time_ms();
			std::cout.flush();
		}
		return 0;
	}

	size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}
}
