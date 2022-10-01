//
// Created by moss on 9/30/22.
//
#include "episode_tests.h"

namespace dropout_dl {
    tests test_episode_name_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = episode::get_episode_name;

        std::string base_test_solution = "Base Test Title";
        std::string base_test = " <h1 class=\"video-title\">\n"
                                "        <strong>" + base_test_solution + "</strong>\n"
                                                                          "      </h1>";

        out.emplace_back("Basic Episode Name Parsing", test_function, base_test, base_test_solution);


        std::string multiple_header_test_solution = "Multi Header Test Title";
        std::string multiple_header_test = "<h1>\n"
                                           "Header without class or strong\n"
                                           "</h1>\n"
                                           "<h1 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small collection-title\">\n"
                                           "Header with incorrect classes"
                                           "</h1>\n"
                                           "<h1>\n"
                                           "<strong>Header with strong</strong>"
                                           "</h1>\n"
                                           "<h1 class=\"video-title\">\n"
                                           "        <strong>" + multiple_header_test_solution + "</strong>\n"
                                                                                                "</h1>\n"
                                                                                                "<h1 class=\"video-title\">\n"
                                                                                                "    <strong> Valid Header and Strong After Correct Title </strong>\n"
                                                                                                "</h1>";


        out.emplace_back("Multiple Header Episode Name Parsing", test_function, multiple_header_test, multiple_header_test_solution);

        std::string no_valid_header_test_solution = "ERROR";
        std::string no_valid_header_test = "<h1>\n"
                                           "Header without class or Strong\n"
                                           "</h1>\n"
                                           "<h1 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small collection-title\">\n"
                                           "Header with incorrect classes"
                                           "</h1>\n"
                                           "<h1>\n"
                                           "<strong>Header with strong</strong>"
                                           "</h1>\n";

        out.emplace_back("No Valid Header Episode Name Parsing", test_function, no_valid_header_test, no_valid_header_test_solution);


        std::string html_character_test_solution = "'&;";
        std::string html_character_test = "<h1 class=\"video-title\">\n"
                                          "        <strong>&#39;&#38;&#59;</strong>\n"
                                          "</h1>";

        out.emplace_back("Html Character Code Episode Name Parsing", test_function, html_character_test, html_character_test_solution);


        return {"Episode Name Parsing", out};
    }

    tests test_episode_number_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = episode::get_episode_number;

        std::string base_test_solution = "1";
        std::string base_test = "<a>\n"
                                "    Season 1, Episode 1\n"
                                "</a>";

        out.emplace_back("Basic Episode Number Parsing", test_function, base_test, base_test_solution);


        std::string multiple_link_test_solution = "1";
        std::string multiple_link_test = "<a>\n"
                                         "asjdhgaorihg\n"
                                         "</a>\n"
                                         "<a>\n"
                                         "    Season 1, Episode 1\n"
                                         "</a>\n"
                                         "<a>\n"
                                         "    Season 1, Episode 2\n";


        out.emplace_back("Multiple Link Episode Number Parsing", test_function, multiple_link_test, multiple_link_test_solution);

        std::string no_valid_number_test_solution = "-1";
        std::string no_valid_number_test = "<a>\n"
                                           "816\n"
                                           "</a>\n"
                                           "<a href=\"www.mossx.net\">\n"
                                           "157"
                                           "</a>\n"
                                           "<a>\n"
                                           "Episode"
                                           "</a>\n";

        out.emplace_back("No Valid Episode Number Parsing", test_function, no_valid_number_test, no_valid_number_test_solution);


        std::string earlier_episode_text_test_solution = "15";
        std::string earlier_episode_text_test = " <h1 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small collection-title video-title\">\n"
                                                "        <strong>Episode Wrong</strong>\n"
                                                "      </h1>\n"
                                                "<a>\n"
                                                "    Season 1, Episode 15\n"
                                                "</a>";

        out.emplace_back("Earlier Episode Text Number Parsing", test_function, earlier_episode_text_test, earlier_episode_text_test_solution);

        return {"Episode Name Parsing", out};
    }

    tests test_episode_series_name_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = episode::get_series_name;

        std::string base_test_solution = "Base Test Title";
        std::string base_test = "<h3 class=\"series-title\">\n"
                                "   <a>\n"
                                "       Base Test Title\n"
                                "   </a>\n"
                                "</h3>";

        out.emplace_back("Basic Episode Series Name Parsing", test_function, base_test, base_test_solution);


        std::string multiple_header_test_solution = "Multi Header Test Title";
        std::string multiple_header_test = "<h3>\n"
                                           "Header without class or link\n"
                                           "</h3>\n"
                                           "<h3 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small collection-title\">\n"
                                           "Header with incorrect classes"
                                           "</h3>\n"
                                           "<h3>\n"
                                           "<a>Header with strong</a>"
                                           "</h3>\n"
                                           "<h3 class=\"series-title\">\n"
                                           "        <a>" + multiple_header_test_solution + "</a>\n"
                                                                                                "</h3>\n"
                                                                                                "<h3 class=\"series-title\">\n"
                                                                                                "    <a> Valid Header and Link After Correct Title </a>\n"
                                                                                                "</h3>";


        out.emplace_back("Multiple Header Episode Series Name Parsing", test_function, multiple_header_test, multiple_header_test_solution);

        std::string no_valid_header_test_solution = "ERROR";
        std::string no_valid_header_test = "<h3>\n"
                                           "Header without class or link\n"
                                           "</h3>\n"
                                           "<h3 class=\"head primary site-font-primary-color site-font-primary-family margin-bottom-small collection-title\">\n"
                                           "Header with incorrect classes"
                                           "</h3>\n"
                                           "<h3>\n"
                                           "<a>Header with strong</a>"
                                           "</h3>\n";

        out.emplace_back("No Valid Header Episode Series Name Parsing", test_function, no_valid_header_test, no_valid_header_test_solution);


        std::string html_character_test_solution = "'&;";
        std::string html_character_test = "<h3 class=\"series-title\">\n"
                                          "        <a>&#39;&#38;&#59;</a>\n"
                                          "</h3>";

        out.emplace_back("Html Character Code Episode Series Name Parsing", test_function, html_character_test, html_character_test_solution);

        return {"Episode Name Parsing", out};
    }

    tests test_episode_embedded_url_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = episode::get_embed_url;

        std::string base_test_solution = "Base Test URL";
        std::string base_test = "    window.VHX.config = {\n"
                                "      embed_url: \"" + base_test_solution + "\"\n"
                                "    };";

        out.emplace_back("Basic Episode Embedded URL Parsing", test_function, base_test, base_test_solution);


        std::string multiple_script_test_solution = "Multi Header Test Title";
        std::string multiple_script_test = "  <script>\n"
                                           "    window.VHX = window.VHX || {};\n"
                                           "    window.VHX.config = {\n"
                                           "      api_url: \"\",\n"
                                           "      token: \"\",\n"
                                           "      token_expires_in: \"\",\n"
                                           "      user_has_subscription: \"\"\n"
                                           "    };\n"
                                           "  </script>"
                                           ""
                                           ""
                                           "<script>\n"
                                           "    window.COMMENTABLE_ID = ;\n"
                                           "    window.VHX = window.VHX || {};\n"
                                           "    window.VHX.config = {\n"
                                           "      api_url: \"\",\n"
                                           "      embed_url: \"" + multiple_script_test_solution + "\",\n"
                                           "      token_expires_in: \"\",\n"
                                           "      user_has_subscription: \"\",\n"
                                           "      is_avod: ,\n"
                                           "      user_has_plan: ,\n"
                                           "      is_admin: \n"
                                           "    };\n"
                                           "    window.VHX.video = {\n"
                                           "      id: \"\",\n"
                                           "      status: \"\",\n"
                                           "      isLive: \"\",\n"
                                           "      scheduled_at: \"\",\n"
                                           "      isDRM: \"\",\n"
                                           "      isTrailer: \"\",\n"
                                           "      \n"
                                           "    };\n"
                                           "  </script>"
                                           ""
                                           ""
                                           "<script>\n"
                                           "    window.COMMENTABLE_ID = 2206938;\n"
                                           "    window.VHX = window.VHX || {};\n"
                                           "    window.VHX.config = {\n"
                                           "      api_url: \"https://api.vhx.tv\",\n"
                                           "      embed_url: \"Correct Setup After Expected Result\",\n"
                                           "      token_expires_in: \"\",\n"
                                           "      user_has_subscription: \"\",\n"
                                           "      is_avod: ,\n"
                                           "      user_has_plan: ,\n"
                                           "      is_admin: \n"
                                           "    };\n"
                                           "    window.VHX.video = {\n"
                                           "      id: \"\",\n"
                                           "      status: \"\",\n"
                                           "      isLive: \"\",\n"
                                           "      scheduled_at: \"\",\n"
                                           "      isDRM: \"\",\n"
                                           "      isTrailer: \"\",\n"
                                           "      \n"
                                           "    };\n"
                                           "  </script>";


        out.emplace_back("Multiple Script Embedded URL Parsing", test_function, multiple_script_test, multiple_script_test_solution);

        std::string no_valid_URL_test_solution = "";
        std::string no_valid_URL_test = "  <script>\n"
                                           "    window.VHX = window.VHX || {};\n"
                                           "    window.VHX.config = {\n"
                                           "      api_url: \"\",\n"
                                           "      token: \"\",\n"
                                           "      token_expires_in: \"\",\n"
                                           "      user_has_subscription: \"\"\n"
                                           "    };\n"
                                           "  </script>";

        out.emplace_back("No Valid Embedded URL Parsing", test_function, no_valid_URL_test, no_valid_URL_test_solution);

        return {"Episode Name Parsing", out};
    }

    tests test_episode_config_url_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

        std::string (*test_function)(const std::string&) = episode::get_config_url;

        std::string base_test_solution = "Base Test URL";
        std::string base_test = R"(window.OTTData = {"config_url":")" + base_test_solution +  "\"};";

        out.emplace_back("Basic Episode Config URL Parsing", test_function, base_test, base_test_solution);


        std::string no_valid_URL_test_solution = "";
        std::string no_valid_URL_test = R"(window.OTTData = {"api_data":{"api_host":"","api_token":"","user_auth_token":{"auth_user_token":"","embed_referrer_host":""}},"buy_button":{"label":null,"url":null},"collection":{"id":null},"product":{"id":null},"":{"label":"","url":""},"site":{"id":null,"subdomain":"","twitter_name":""},"video":{"duration":null,"id":null,"is_trailer":"","title":"","is_live_video":false,"live_event_id":null},"google_cast_app_id":"","hide_chrome":null,"initial_time":null,"js_api_enabled":null,"locale":"","show_share_actions":null,"user":{"id":,"email":""},"analytics_url":""})";

        out.emplace_back("No Valid Config URL Parsing", test_function, no_valid_URL_test, no_valid_URL_test_solution);

        return {"Episode Name Parsing", out};
    }
}

std::vector<dropout_dl::tests> test_episode() {

    std::vector<dropout_dl::tests> testss;

    testss.push_back(dropout_dl::test_episode_name_parsing());


    testss.push_back(dropout_dl::test_episode_number_parsing());


    testss.push_back(dropout_dl::test_episode_series_name_parsing());


    testss.push_back(dropout_dl::test_episode_embedded_url_parsing());


    testss.push_back(dropout_dl::test_episode_config_url_parsing());


    return testss;
}