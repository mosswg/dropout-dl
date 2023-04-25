#pragma once
#include <iostream>

#ifdef DROPOUT_DL_SQLITE
#include <sqlite3.h>
#endif

#include "util.h"

namespace dropout_dl {
	class cookie {
	public:
		/**
		 *
		 * @param data - A string to write to return value of the command to
		 * @param argc - The number of results from the command
		 * @param argv - The results of the command
		 * @param azColName - The column names from the command
		 * @return 0 on success or -1 on failure
		 *
		 * Used by sqlite. Write the first result of the sqlite command to the data string. If there are no results print an error and return -1.
		 */
		static int sqlite_write_callback(void* data, int argc, char** argv, char** azColName)
		{
			if (argc < 1) {
				std::cerr << "SQLITE ERROR: sqlite could not find desired cookie" << std::endl;
				return -1;
			}
			else {
				*(std::string*)data = argv[0];
				return 0;
			}
		}

		/**
		 * The name of the value from the sqlite database or "?" if not set.
		 */
		std::string name;
		/**
		 * The value of the cookie
		 */
		std::string value;
		/**
		 * The length of the value of the cookie
		 */
		int len;


		/**
		 *
		 * Create a cookie with no name, value, or length.
		 */
		cookie() {
			this->name = "";
			this->value = "";
			this->len = 0;
		}

		/**
		 *
		 * @param name - Name of the value from the sqlite database
		 *
		 * Create a cookie with no value and length of 0
		 */
		explicit cookie(const std::string& name) {
			this->name = name;
			this->value = "";
			this->len = 0;
		}

		/**
		 *
		 * @param name - Name of the value from the sqlite database
		 * @param cookie - Value of the cookie
		 *
		 * Sets the name and value using the parameters and gets the length from the value
		 */
		cookie(const std::string& name, const std::string& cookie) {
			this->name = name;
			this->value = cookie;
			this->len = cookie.size();
		}

		/**
		 *
		 * @param cookie - Value of the cookie
		 * @param length - Length of the cookie
		 *
		 * Sets the value and length using the parameters and sets the name as "?"
		 */
		cookie(const std::string& cookie, int length) {
			this->value = cookie;
			this->name = "?";
			this->len = length;
		}

		/**
		 *
		 * @param name - Name of the value from the sqlite database
		 * @param cookie - Value of the cookie
		 * @param length - Length of the cookie
		 *
		 * Sets the name, value, and length using the parameters leaving nothing unset.
		 */
		cookie(const std::string& name, const std::string& cookie, int length) {
			this->name = name;
			this->value = cookie;
			this->len = length;
		}


#ifdef DROPOUT_DL_SQLITE
		/**
		 *
		 * @param db - An sqlite3 database
		 * @param sql_query_base - A base without the name search e.g. "FROM cookies" this function would then append the text "SELECT <value>" and "WHERE name='<name>'"
		 * @param value - The name of the value to fill the cookie with
		 *
		 * Retrieve the value of a cookie from the provided sqlite database.
		 *
		 */
		void get_value_from_db(sqlite3* db, const std::string& sql_query_base, const std::string& value, bool verbose = false, int (*callback)(void*,int,char**,char**) = sqlite_write_callback);
#endif

		/**
		 *
		 * @param password - Default is "peanuts". This works for linux. The password should be keychain password on MacOS
		 * @param salt - Salt is "saltysalt" for both MacOS and Linux
		 * @param length - Length of 16 is standard for both MacOS and Linux
		 * @param iterations - 1 on linux and 1003 on MacOS
		 *
		 * Decrypt chrome cookies and format them to be usable as regular cookies. Currently this has only been tested for the _session and __cf_bm cookies from dropout.tv but I see no reason this would not work for anything else.
		 */
		void chrome_decrypt(const std::string& password = "peanuts", int iterations = 1, const std::string& salt = "saltysalt", int length = 16);

	private:
		/**
		 * Remove url encoded text from a cookie. Currently this only checks for %3D ('=') as that is the only thing I've come across in cookies during the entirety of this project.
		 */
		void url_decode();

		/**
		 * Remove the leading version (e.g. "v10") from the cookie
		 */
		void format_from_chrome();
	};
}
