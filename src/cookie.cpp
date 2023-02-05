#include "cookie.h"


// Cookie functions
namespace dropout_dl {
	void cookie::get_value_from_db(sqlite3 *db, const std::string &sql_query_base, const std::string& value, bool verbose, int (*callback)(void*,int,char**,char**)) {
#ifdef DROPOUT_DL_SQLITE
		std::string sql_mod_base = sql_query_base;

		if (sql_mod_base.find("WHERE") == std::string::npos) {
			sql_mod_base += " WHERE ";
		}
		else {
			sql_mod_base += " AND ";
		}

		sql_mod_base += "name='" + this->name + "';";

		std::string sql_value_query = "SELECT " + value + ' ' + sql_mod_base;
		std::string sql_length_query = "SELECT length(" + value + ") " + sql_mod_base;
		std::string tmp;
		char *err_code = nullptr;
		int rc;

		if (verbose) {
			std::cout << sql_value_query << '\n' << sql_length_query << std::endl;
		}

		rc = sqlite3_exec(db, sql_length_query.c_str(), callback, &tmp, &err_code);

		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", err_code);
			sqlite3_free(err_code);
			sqlite3_close(db);
			exit(3);
		} else if (verbose) {
			std::cout << "Got " << this->name << " cookie length\n";
		}

		if (tmp.empty()) {
			std::cerr << "COOKIE SQLITE ERROR: No Cookie With Name " << this->name << " Exists\n";
			exit(0);
		}
		this->len = std::stoi(tmp);

		rc = sqlite3_exec(db, sql_value_query.c_str(), callback, &tmp, &err_code);

		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", err_code);
			sqlite3_free(err_code);
			sqlite3_close(db);
			exit(3);
		} else if (verbose) {
			std::cout << "Got " << this->name << " cookie\n";
		}

		this->value = tmp;
#else
		std::cerr << "COOKIE ERROR: Attempted to get cookies from sqlite without having sqlite installed\n";
		exit(12);
#endif
	}

	void cookie::format_from_chrome() {
		this->value = this->value.substr(3);
		this->len -= 3;
	}

	void cookie::chrome_decrypt(const std::string &password, int iterations, const std::string &salt, int length) {

#ifdef DROPOUT_DL_GCRYPT
		this->format_from_chrome();

		uint8_t key[32];

		char output[this->len + 2];

		char iv[16];

		for (char& c : iv) {
			c = ' ';
		}

		for (char& c : output) {
			c = 0;
		}

		gcry_kdf_derive(password.c_str(), password.size(), GCRY_KDF_PBKDF2, GCRY_KDF_ARGON2ID, salt.c_str(), salt.size(), iterations, length, key);

		gcry_cipher_hd_t handle;

		gcry_cipher_open(&handle, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);

		gcry_cipher_setkey(handle, (const void*) &key, length);

		gcry_cipher_setiv(handle, (const void*)&iv, 16);

		unsigned long err = gcry_cipher_decrypt(handle, output, this->len, this->value.c_str(), this->len);

		if (err) {
			std::cout << gcry_strerror(err) << std::endl;
			exit(2);
		}


		this->value = output;

		this->url_decode();

		this->value = this->value.substr(0, this->len - 7);
		this->len -= 7;
#else
		std::cerr << "CHROME COOKIE ERROR: Attempted to Decrypt Chrome Cookie Without libgcrypt\n";
		exit(12);
#endif
	}

	void cookie::url_decode() {
		std::string out;

		for (int i = 0; i < this->value.size() - 3; i++) {
			if (substr_is(this->value, i, "%3D")) {
				out += "=";
				i += 2;
			}
			else {
				out += this->value[i];
			}
		}

		this->value = out;
		this->len = out.size();
	}

}
