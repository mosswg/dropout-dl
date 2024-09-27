#include "util.h"
#include "color.h"
#include <stdint.h>
#include <regex>

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

			switch(c) {
				//Ignored characters
				case '?':
				case ':':
				case '\\':
				case '\'':
				case '(':
				case ')':
					continue;
				//Characters converted into dashes
				case '/':
					out += '-';
					break;
				default:
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

	void segment_progress_func(std::string filename, int num_done, int total) {
		const double number_chars = 50;
		const char* full_character = "▓";
		const char* empty_character = "░";

		current_time = time_ms();
		if (current_time - 50 > last_progress_timestamp) {
			// Percent of the file downloaded. Adding one to round up so that when its done it shows as a full bar rather than missing one.
			double percent_done = (((double)num_done / (double)total) * number_chars) + 1;
			double percent_done_clone = percent_done;
			std::cout << filename << " [";
			while (percent_done_clone-- > 0) {
				std::cout << full_character;
			}
			while (percent_done++ < number_chars) {
				std::cout << empty_character;
			}
			std::cout << "] " << num_done << " / " << total << " Segments                                 ";
			putchar('\r');
			last_progress_timestamp = time_ms();
			std::cout.flush();
		}
	}

	size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		if(userp == nullptr)
			std::cerr << RED << "ERROR: WriteCallBack failed to write to output buffer :(\n";
		else
			((std::string*)userp)->append((char*)contents, size * nmemb);
		if(!size)
			std::cout << YELLOW << "WARN: Packet had no data :(\n";
		return size * nmemb;
	}


	std::string get_generic_page(const std::string& url, long* response_status, bool verbose, std::string* header_string) {
		CURL *hnd;
		struct curl_slist *slist1;

		std::string page_data;

		slist1 = nullptr;
		slist1 = curl_slist_append(slist1, "Accept: text/html");
		slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en");
		slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
		slist1 = curl_slist_append(slist1, "DNT: 1");
		slist1 = curl_slist_append(slist1, "Connection: keep-alive");
		slist1 = curl_slist_append(slist1, "Referer: https://www.dropout.tv/");
		slist1 = curl_slist_append(slist1, "Upgrade-Insecure-Requests: 1");
		slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: iframe");
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
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &page_data);


		if (header_string) {
			curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, WriteCallback);
			curl_easy_setopt(hnd, CURLOPT_HEADERDATA, header_string);
		}

		curl_easy_perform(hnd);

		if (response_status) {
			curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, response_status);
		}

		curl_easy_cleanup(hnd);
		hnd = nullptr;
		curl_slist_free_all(slist1);
		slist1 = nullptr;

		return page_data;
	}

	size_t EmptyWriteCallback(void*, size_t, size_t, void*) {
		// This callback function is needed for CURLOPT_NOBODY to discard the response body.
		return 0;
	}


	std::string get_substring_in(const std::string& string, const std::string& begin, const std::string& end, int starting_index) {
		size_t substring_start = string.find(begin, starting_index);

		if (substring_start == std::string::npos) {
			std::cerr << "ERROR: Could not find start of substring\n";
			return "";
		}

		// Skip over the contents of 'begin'
		substring_start += begin.size();

		size_t substring_end = string.find(end, substring_start);

		if (substring_end == std::string::npos) {
			std::cerr << "ERROR: Could not find end of substring\n";
			return "";
		}


		return string.substr(substring_start, substring_end - substring_start);
	}


	// https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
	std::string url_encode(const std::string& value) {
		static auto hex_digt = "0123456789ABCDEF";

		std::string result;
		result.reserve(value.size() << 1);

		for (auto ch : value)
			{
				if ((ch >= '0' && ch <= '9')
					|| (ch >= 'A' && ch <= 'Z')
					|| (ch >= 'a' && ch <= 'z')
					|| ch == '-' || ch == '_' || ch == '!'
					|| ch == '\'' || ch == '(' || ch == ')'
					|| ch == '*' || ch == '~' || ch == '.')  // !'()*-._~
					{
						result.push_back(ch);
					}
				else
					{
						result += std::string("%") +
							hex_digt[static_cast<unsigned char>(ch) >> 4]
							+  hex_digt[static_cast<unsigned char>(ch) & 15];
					}
			}

		return result;
	}
	int get_int_in_string(const std::string& str, int& starting_index)  {
		std::smatch string_number;
		if(!std::regex_search(str,string_number,std::regex("-?\\d+"))) {
			std::cerr << YELLOW << "WARN: Unable to find number in string '" << str << "'!\n" << RESET;
			return 0;
		}
        starting_index = string_number.position();
		return std::stoi(string_number[0]);
    }
	int get_int_in_string(const std::string& string) {
		int temp;
		return get_int_in_string(string,temp); // lol
	}

	int get_month_string_as_int(const std::string& month) {
		// NOTE: We probably could've used <chrono> here but~ that header's pretty big so this is probably faster to compile :^)
		static std::vector<std::string> month_strings {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
		for(int i = 0; i < 12; ++i)
			if(month_strings[i] == month) return i;
		return -1;
	}


// base64 code is take from http://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/
	unsigned int base64_pos_of_char(const unsigned char chr) {
		//
		// Return the position of chr within base64_encode()
		//

		if      (chr >= 'A' && chr <= 'Z') return chr - 'A';
		else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A')               + 1;
		else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
		else if (chr == '+' || chr == '-') return 62; // Be liberal with input and accept both url ('-') and non-url ('+') base 64 characters (
		else if (chr == '/' || chr == '_') return 63; // Ditto for '/' and '_'
		else
			//
			// 2020-10-23: Throw std::exception rather than const char*
			//(Pablo Martin-Gomez, https://github.com/Bouska)
			//
			throw std::runtime_error("Input is not valid base64-encoded data.");
	}

	std::string base64_decode(const std::string& encoded_string) {
		//
		// decode(…) is templated so that it can be used with String = const std::string&
		// or std::string_view (requires at least C++17)
		//

		if (encoded_string.empty()) return std::string();

		size_t length_of_string = encoded_string.length();
		size_t pos = 0;

		//
		// The approximate length (bytes) of the decoded string might be one or
		// two bytes smaller, depending on the amount of trailing equal signs
		// in the encoded string. This approximation is needed to reserve
		// enough space in the string to be returned.
		//
		size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
		std::string ret;
		ret.reserve(approx_length_of_decoded_string);

		while (pos < length_of_string) {
			// Iterate over encoded input string in chunks. The size of all
			// chunks except the last one is 4 bytes.
			//
			// The last chunk might be padded with equal signs or dots
			// in order to make it 4 bytes in size as well, but this
			// is not required as per RFC 2045.
			//
			// All chunks except the last one produce three output bytes.
			//
			// The last chunk produces at least one and up to three bytes.
			size_t pos_of_char_1 = base64_pos_of_char(encoded_string.at(pos+1) );

			//
			// Emit the first output byte that is produced in each chunk:
			//
			ret.push_back(static_cast<std::string::value_type>(((base64_pos_of_char(encoded_string.at(pos+0))) << 2) + ((pos_of_char_1 & 0x30 ) >> 4)));

			if ((pos + 2 < length_of_string) &&  // Check for data that is not padded with equal signs (which is allowed by RFC 2045)
				encoded_string.at(pos+2) != '=' &&
				encoded_string.at(pos+2) != '.' // accept URL-safe base 64 strings, too, so check for '.' also.
				) {
				//
				// Emit a chunk's second byte (which might not be produced in the last chunk).
				//
				unsigned int pos_of_char_2 = base64_pos_of_char(encoded_string.at(pos+2));
				ret.push_back(static_cast<std::string::value_type>( (( pos_of_char_1 & 0x0f) << 4) + (( pos_of_char_2 & 0x3c) >> 2)));

				if ((pos + 3 < length_of_string) &&
					encoded_string.at(pos+3) != '=' &&
					encoded_string.at(pos+3) != '.') {
					//
					// Emit a chunk's third byte (which might not be produced in the last chunk).
					//
					ret.push_back(static_cast<std::string::value_type>(((pos_of_char_2 & 0x03) << 6) + base64_pos_of_char(encoded_string.at(pos+3))));
				}
			}

			pos += 4;
		}

		return ret;
	}

}
