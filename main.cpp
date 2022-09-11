#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <curl/curl.h>


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int clear_icanon()
{
    struct termios settings{};
    int result;
    result = tcgetattr (STDIN_FILENO, &settings);
    if (result < 0)
    {
        perror ("error in tcgetattr");
        return 0;
    }

    settings.c_lflag &= ~ICANON;

    result = tcsetattr (STDIN_FILENO, TCSANOW, &settings);
    if (result < 0)
    {
        perror ("error in tcsetattr");
        return 0;
    }
    return 1;
}

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

int progress_func(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    std::cout << (int)NowDownloaded << "/" << (int)TotalToDownload << std::endl;
    return 0;
}

std::string get_series_name(const std::string& html_data) {
    std::string series_title("series-title");
    std::string open_a_tag("<a");
    std::string close_tag(">");
    std::string close_a("</a>");

    int series_name_start = -1;

    for (int i = 0; i < html_data.size(); i++) {
        if (substr_is(html_data, i, series_title)) {
            for (int j = i + series_title.size(); j < html_data.size(); j++) {
                if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                if (substr_is(html_data, j, open_a_tag)) {
                    for (int k = j + open_a_tag.size(); k < html_data.size(); k++) {
                        if (substr_is(html_data, k, close_tag)) {
                            for (int l = 0; l < html_data.size() - k; l++) {
                                char c = html_data[k + l];
                                if (series_name_start == -1) {
                                    if (html_data[k + l + 1] == '\n' || html_data[k + l + 1] == ' ' ||
                                        html_data[k + l + 1] == '\t') {
                                        continue;
                                    } else {
                                        series_name_start = k + l + 1;
                                    }
                                }
                                if (substr_is(html_data, k + l, close_a) || (series_name_start != -1 && html_data[k + l] == '\n')) {
                                    return html_data.substr(series_name_start, l - (series_name_start - k));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return "-1";
}

std::string get_episode_name(const std::string& html_data) {
    int title_start = -1;
    std::string video_title("video-title");
    std::string open_strong("<strong>");
    std::string close_strong("</strong>");
    for (int i = 0; i < html_data.size(); i++) {
        if (substr_is(html_data, i, video_title)) {
            for (int j = i; j < html_data.size(); j++) {
                if (substr_is(html_data, j, open_strong)) {
                    title_start = j + 8;
                    break;
                }
            }
            for (int j = 0; j < html_data.size() - title_start; j++) {
                if (substr_is(html_data, title_start + j, close_strong)) {
                    return html_data.substr(title_start, j);
                }
            }
        }
    }
    return "ERROR";
}

std::string get_episode_number(const std::string& html_data) {
    std::string episode("Episode");
    std::string close_a("</a>");
    std::string episode_num;
    for (int i = 0; i < html_data.size(); i++) {
        if (substr_is(html_data, i, episode)) {
            for (int j = i + 8; j < html_data.size(); j++) {
                if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                if (substr_is(html_data, j, close_a)) {
                    return episode_num;
                }
                episode_num += html_data[j];
            }
        }
    }
    return "-1";
}

std::string get_season_number(const std::string& html_data) {
    std::string season("Season");
    std::string dash(",");
    std::string season_num;
    for (int i = 0; i < html_data.size(); i++) {
        if (substr_is(html_data, i, season)) {
            for (int j = i + 7; j < html_data.size(); j++) {
                if (html_data[j] == '\n' || html_data[j] == ' ' || html_data[j] == '\t') continue;
                if (html_data[j] == '-' || html_data[j] == ',' ) {
                    return season_num;
                }
                season_num += html_data[j];
            }
        }
    }
    return "-1";
}

std::string get_embed_url(const std::string& html_data) {
    std::string config("window.VHX.config");
    std::string embed_url("embed_url: ");
    std::string dash(",");
    std::string season_num;
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

std::string get_config_url(const std::string& html_data) {
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

std::string get_embedded_page(const std::string& url) {
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1;
        std::string embedded_page;

        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:101.0) Gecko/20100101 Firefox/101.0");
        slist1 = curl_slist_append(slist1, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
        slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en;q=0.5");
        slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
        slist1 = curl_slist_append(slist1, "DNT: 1");
        slist1 = curl_slist_append(slist1, "Connection: keep-alive");
        slist1 = curl_slist_append(slist1, "Referer: https://www.dropout.tv/");
        slist1 = curl_slist_append(slist1, "Cookie: __cf_bm=Ayc3uSgUEf9kJ20sfVBLgdo5fvloLmSLWBkJtzzhZR8-1662831290-0-ASVO2Fg9txI6nslt2tle7Y2MjRw4sI8/gFRbMDI8vHIP0nhb1SDk1I7lF5hWK9RMGP9wOFJwyqThLXQkuTj9m2c=");
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
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.84.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);


        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &embedded_page);
        /* Here is a list of options the curl code used that cannot get generated
           as source easily. You may choose to either not use them or implement
           them yourself.

        CURLOPT_WRITEDATA set to a objectpointer
        CURLOPT_INTERLEAVEDATA set to a objectpointer
        CURLOPT_WRITEFUNCTION set to a functionpointer
        CURLOPT_READDATA set to a objectpointer
        CURLOPT_READFUNCTION set to a functionpointer
        CURLOPT_SEEKDATA set to a objectpointer
        CURLOPT_SEEKFUNCTION set to a functionpointer
        CURLOPT_ERRORBUFFER set to a objectpointer
        CURLOPT_STDERR set to a objectpointer
        CURLOPT_HEADERFUNCTION set to a functionpointer
        CURLOPT_HEADERDATA set to a objectpointer

        */

        ret = curl_easy_perform(hnd);

        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;

        return embedded_page;
    }

std::string get_config_page(const std::string& url) {
    CURLcode ret;
    CURL *hnd;
    struct curl_slist *slist1;

    std::string config_page;

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:101.0) Gecko/20100101 Firefox/101.0");
    slist1 = curl_slist_append(slist1, "Accept: */*");
    slist1 = curl_slist_append(slist1, "Accept-Language: en-US,en;q=0.5");
    slist1 = curl_slist_append(slist1, "Accept-Encoding: utf-8");
    slist1 = curl_slist_append(slist1, "Referer: https://embed.vhx.tv/");
    slist1 = curl_slist_append(slist1, "Origin: https://embed.vhx.tv");
    slist1 = curl_slist_append(slist1, "DNT: 1");
    slist1 = curl_slist_append(slist1, "Connection: keep-alive");
    slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: empty");
    slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: cors");
    slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
    slist1 = curl_slist_append(slist1, "Sec-GPC: 1");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.84.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &config_page);

    /* Here is a list of options the curl code used that cannot get generated
       as source easily. You may choose to either not use them or implement
       them yourself.

    CURLOPT_WRITEDATA set to a objectpointer
    CURLOPT_INTERLEAVEDATA set to a objectpointer
    CURLOPT_WRITEFUNCTION set to a functionpointer
    CURLOPT_READDATA set to a objectpointer
    CURLOPT_READFUNCTION set to a functionpointer
    CURLOPT_SEEKDATA set to a objectpointer
    CURLOPT_SEEKFUNCTION set to a functionpointer
    CURLOPT_ERRORBUFFER set to a objectpointer
    CURLOPT_STDERR set to a objectpointer
    CURLOPT_HEADERFUNCTION set to a functionpointer
    CURLOPT_HEADERDATA set to a objectpointer

    */

    ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return config_page;
}

int main(int argc, char** argv) {

    clear_icanon(); // Changes terminal from canonical mode to non canonical mode.

    std::string episode_url;
    std::string series_name;
    std::string name;
    std::string filename;
    std::string season;
    std::string episode;
    std::string config_url;
    std::string embed_url;

    std::string cookie("cookie: __cf_bm=0.kYAdtpkhPsBzc7110IISrk0ZK1Hz5etmUggs6Z00g-1662827485-0-AeyJKQvZu3FJU+Y8wWRFuztF1Se6UDldh+o1OG/B72x/LZF+tsXOT7xs20Xmv4//kLDU53Oa05hQKbR3/dwMW2k=");
    std::string user_auth_token("auth-user-token=eyJhbGciOiJIUzI1NiJ9.eyJ1c2VyX2lkIjo0NzM2MDk3OSwiZXhwIjoxNjYyODQwNjcwfQ.J0Jw2g2PLlLYQziRUbkvQwxIvg9SCGqtsROJbMB1LHE");

    CURL *curl;
    CURLcode res;
    std::string episode_data;
    std::string embedded_data;
    std::string config_data;
    std::string video_data;

    if (argc > 1) {
        episode_url = argv[2];
    }
    else {
        std::cout << "Enter episode url: ";

        std::cin >> episode_url;
    }

    curl = curl_easy_init();
    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, episode_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &episode_data);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        name = get_episode_name(episode_data);

        season = get_season_number(episode_data);

        episode = get_episode_number(episode_data);

        series_name = get_series_name(episode_data);

        std::replace(series_name.begin(), series_name.end(), ' ', '_');

        std::replace(series_name.begin(), series_name.end(), ',', '_');

        filename = series_name + "/S" + (season.size() < 2 ? "0" + season : season) + "E" + (episode.size() < 2 ? "0" + episode : episode) + name + ".mp4";

        std::replace(filename.begin(), filename.end(), ' ', '_');

        std::replace(filename.begin(), filename.end(), ',', '_');


        embed_url = get_embed_url(episode_data);

        replace_all(embed_url, "&amp;", "&");

        embed_url.insert(embed_url.find("?api=1&") + 7, user_auth_token + "&");

        std::cout << std::endl << "embed url: " << embed_url << std::endl;
    }

    curl = curl_easy_init();
    if (curl) {
        embedded_data = get_embedded_page(embed_url);

        config_url = get_config_url(embedded_data);

        replace_all(config_url, "\\u0026", "&");

        config_data = get_config_page(config_url);
    }

    int i = 0;
    bool video_section = false;
    for (; i < config_data.size(); i++ ) {
        // std::cout << i << "/" << javascript_data.size() << ": " << javascript_data[i] << ": " << javascript_data.substr(i, 17) << ": " << video_section << "\n";
        if (config_data.substr(i, 3) == "mp4") {
            video_section = true;
        }

        if (video_section && config_data.substr(i, 17) == R"("quality":"1080p")") {
            std::cout << config_data.substr(i) << '\n';
            break;
        }
    }
    if (i == config_data.size() - 1) {
        std::cout << "quality of 1080p not found" << std::endl;
        exit(1);
    }

    std::string url;
    for (; i > 0; i--) {
        // std::cout << i << ": " << javascript_data[i] << ": " << javascript_data.substr(i-7, 7) << "\n";
        if (config_data.substr(i-7, 7) == R"("url":")") {
            std::cout << config_data.substr(i) << '\n';
            break;
        }
    }

    while (config_data[i] != '"') {
        url += config_data[i++];
    }

    curl = curl_easy_init();
    if(curl) {
        if (!std::filesystem::is_directory(series_name)) {
            std::filesystem::create_directories(series_name);
        }

        std::fstream out(filename, std::ios_base::in|std::ios_base::out|std::ios_base::trunc);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &video_data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
        std::cout << "getting \"" << filename << " from " << url << std::endl;
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        out << video_data << std::endl;
    }
    return 0;
}
