#include "login.h"


void dropout_dl::login::get_cookies(std::string& session) {
	std::string email;
	std::string password;

	const std::string home = getenv("HOME");
	const std::string cache_directory = home + "/.cache/dropout-dl/";
	const std::string cache_file_path = home + "/.cache/dropout-dl/token-cache";
	std::fstream cache_file(cache_file_path);

	/// check if file exist
	if(!cache_file.fail()) {
		std::cout << GREEN << "Using cached tokens from " << cache_file_path << RESET << "\n";
		std::string cached_session_token;
		std::string expiration_date;
		std::getline(cache_file, cached_session_token);
		std::getline(cache_file, expiration_date);

		int day_of_the_month_index = 0;
		int day_of_the_month = get_int_in_string(expiration_date, day_of_the_month_index);

		std::string month = expiration_date.substr(day_of_the_month_index + 1, 3);
		int month_as_int = get_month_string_as_int(month);

		int year_index = day_of_the_month_index + 4;
		int year = get_int_in_string(expiration_date, year_index);

		time_t now = time(0);

		tm *ltm = localtime(&now);

		// If the expiration year is larger than current year.
		if (year > (1900 + ltm->tm_year)) {
			session = cached_session_token;
			return;
		}

		if (year == (1900 + ltm->tm_year)) {
			// If the year is the same and the expiration month is larger
			if (month_as_int > ltm->tm_mon) {
				session = cached_session_token;
				return;
			}

			if (month_as_int == ltm->tm_mon) {
				if (day_of_the_month > ltm->tm_mday) {
					session = cached_session_token;
					return;
				}
			}
		}

		std::cout << YELLOW << "Cached tokens expired" << RESET << "\n";
	}

	std::cout << "Logging in...\n";

	get_login_info_from_file("login", email, password);

	/// Needed to login properly
	std::string authentication;
	std::string cf_bm;
	// Needed for caching
	std::string session_expiration;
	get_login_tokens(session, cf_bm, authentication, session_expiration);

	if (!login_with_tokens(email, password, session, cf_bm, authentication)) {
		std::cerr << RED << "ERROR: Could not login. Check your login. If you are certain your information is correct please report this issue.\n";
		exit(1);
	}

	std::cout << GREEN << "Successfully logged in.\n" << RESET;

	std::cout << "Caching tokens...\r";


	if (!std::filesystem::is_directory(cache_directory)) {
		std::filesystem::create_directories(cache_directory);
	}

	cache_file.open(cache_file_path, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

	cache_file << session << "\n" << session_expiration << "\n";

	std::cout << GREEN "Cached tokens to " << cache_file_path << "\n" << RESET;
}

void dropout_dl::login::get_login_info_from_file(const std::string& filename, std::string& email, std::string& password) {
	std::ifstream login_file(filename);

	if (!login_file) {
		std::cerr << "ERROR: Could not open login file\n";
		exit(1);
	}

	std::getline(login_file, email);
	std::getline(login_file, password);

	if (email.empty() || password.empty()) {
		std::cerr << "ERROR: Invalid login format in file. File must contain just your email then password on seperate lines. Example:\nemail@example.com\npassword123\n";
		exit(1);
	}

	if (email.find("@") == std::string::npos || email.find(".") == std::string::npos) {
		/// Not outputting email because that could potentially reveal password if they are in the opposite place.
		std::cerr << "ERROR: Invalid email in login file\n";
		exit(1);
	}
}

void dropout_dl::login::get_login_tokens(std::string& session_token, std::string& cf_bm_token, std::string& authentication_token, std::string& session_expiration) {
	std::string login_page_url = "https://www.dropout.tv/login";
	std::string header_string = "";

	std::string login_page_data = get_generic_page(login_page_url, nullptr, false, &header_string);

	size_t set_cookie_index = header_string.find("set-cookie: _session=");

	if (set_cookie_index != std::string::npos) {
		session_token = get_substring_in(header_string, "set-cookie: _session=", ";");
		session_expiration = get_substring_in(header_string, "expires=", ";", set_cookie_index);
	}

	cf_bm_token = get_substring_in(header_string, "set-cookie: __cf_bm=", ";");

	authentication_token = get_substring_in(login_page_data, "<meta name=\"csrf-param\" content=\"authenticity_token\" />\n<meta name=\"csrf-token\" content=\"", "\"");
}

bool dropout_dl::login::login_with_tokens(const std::string& email, const std::string& password, std::string& session, const std::string& cf_bm, const std::string& authentication_token) {
	CURLcode ret;
	CURL *hnd;
	struct curl_slist *slist1;

	slist1 = NULL;

	std::string email_encoded = url_encode(email), password_encoded = url_encode(password), authentication_token_encoded = url_encode(authentication_token);

	std::string cookies = "Cookie: locale_det=en; referrer_url=https%3A%2F%2Fwww.dropout.tv%2F; _session=" + session + "; __cf_bm=" + cf_bm;

	slist1 = curl_slist_append(slist1, "Content-Type: application/x-www-form-urlencoded");
	slist1 = curl_slist_append(slist1, "Origin: https://www.dropout.tv");
	slist1 = curl_slist_append(slist1, "Connection: keep-alive");
	slist1 = curl_slist_append(slist1, "Referer: https://www.dropout.tv/login");
	slist1 = curl_slist_append(slist1, cookies.c_str());

	hnd = curl_easy_init();
	std::string postfields = "email=" + email_encoded + "&authenticity_token=" + authentication_token_encoded + "&password=" + password_encoded;
	long http_response_code = 0;
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
	curl_easy_setopt(hnd, CURLOPT_URL, "https://www.dropout.tv/login");
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, postfields.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)postfields.size());
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.87.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	std::string header_string;
	curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, dropout_dl::WriteCallback);
	curl_easy_setopt(hnd, CURLOPT_HEADERDATA, &header_string);

	/// Hide output
	/// TODO
	FILE* nullvoid = fopen("/dev/null", "r");
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, nullvoid);


	ret = curl_easy_perform(hnd);

	// set the session token to the new value in the response header.
	session = get_substring_in(header_string, "set-cookie: _session=", ";");


	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_response_code);

	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;

	/// 302 is returned on sucessful login otherwise 200.
	return http_response_code == 302;
}
