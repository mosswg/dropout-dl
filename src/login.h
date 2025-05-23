#pragma once

#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

#include <curl/curl.h>

#include "cookie.h"
#include "util.h"
#include "color.h"

namespace dropout_dl {
	namespace login {
		void get_cookies(std::string& session, const std::string& login_file);

		void get_login_info_from_file(const std::string& filename, std::string& email, std::string& password);

		void get_login_tokens(std::string& session_token, std::string& cf_bm_token, std::string& authentication_token, std::string& session_expiration);

		/**
		 *
		 * @param email
		 * @param password
		 * @param session - _session cookie. this changes with the response header to the login request.
		 * @param cf_bm - __cf_bm cookie. this does not change.
		 * @param authentication_token - an authentication token that is set on the login page and changes every time. I don't understand the purpose of this.
		 * @return true on successful login. false otherwise.
		 *
		 * Login with the provided tokens and change session token.
		 */
		bool login_with_tokens(const std::string& email, const std::string& password, std::string& session, const std::string& cf_bm, const std::string& authentication_token);
	}
}
