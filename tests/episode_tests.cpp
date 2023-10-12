//
// Created by moss on 9/30/22.
//
#include "episode_tests.h"

namespace dropout_dl {
    tests test_episode_name_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

	/// TODO: Fix this to use episode::get_episode_name with json
        std::string (*test_function)(const std::string&) = nullptr;


		// window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":2429553,"COLLECTION_ID":784936,"COLLECTION_TITLE":"Season 5","PRODUCT_ID":28599,"VIDEO_TITLE":"Sam Says 2","CANONICAL_COLLECTION":{"id":784936,"name":"Season 5","href":"https://www.dropout.tv/season-5-7","parent":{"id":121093,"name":"Game Changer","type":"series"}}}}

        std::string base_test_solution = "Base Test Title";
        std::string base_test = R"(window.Page = {"VIDEO_TITLE":")" + base_test_solution + "\"}";


        out.emplace_back("Basic Episode Name Parsing", test_function, base_test, base_test_solution);


        std::string full_test_solution = "Full Test Title";
        std::string full_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"VIDEO_TITLE":")" + full_test_solution +  R"(","CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"name":"lorem","type":"series"}}}})";

        out.emplace_back("Full Episode Name Parsing", test_function, full_test, full_test_solution);

        std::string no_valid_title_test_solution = "ERROR";
        std::string no_valid_title_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"name":"lorem","type":"series"}}}})";

        out.emplace_back("No Valid Title Episode Name Parsing", test_function, no_valid_title_test, no_valid_title_test_solution);


        std::string html_character_test_solution = "'&;";
        std::string html_character_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"VIDEO_TITLE":"&#39;&#38;&#59;","CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"name":"lorem","type":"series"}}}})";

        out.emplace_back("Html Character Code Episode Name Parsing", test_function, html_character_test, html_character_test_solution);

        return {"Episode Name Parsing", out};
    }

    tests test_episode_series_name_parsing() {
        std::vector<dropout_dl::test<std::string>> out;

	/// TODO: Fix this to use episode::get_series_name with json
        std::string (*test_function)(const std::string&) = nullptr;

		// window.Page = {"PROPERTIES":{"VIEW_TYPE":"collection","PRODUCT_ID":28599,"COLLECTION_ID":121093,"COLLECTION_TITLE":"Game Changer"}}

        std::string base_test_solution = "Base Test Title";
        std::string base_test = R"(window.Page = {"parent":{"name":")" + base_test_solution + R"("}})";

        out.emplace_back("Basic Episode Series Name Parsing", test_function, base_test, base_test_solution);


        std::string full_test_solution = "Full Test Title";
        std::string full_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"VIDEO_TITLE":"lorem","CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"name":")" + full_test_solution + R"(","type":"series"}}}})";

        out.emplace_back("Full Series Name Parsing", test_function, full_test, full_test_solution);

        std::string no_valid_header_test_solution = "ERROR";
        std::string no_valid_header_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"VIDEO_TITLE":"lorem","CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"type":"series"}}}})";
        out.emplace_back("No Valid Header Episode Series Name Parsing", test_function, no_valid_header_test, no_valid_header_test_solution);


        std::string html_character_test_solution = "'&;";
        std::string html_character_test = R"(window.Page = {"PROPERTIES":{"VIEW_TYPE":"video","VIDEO_ID":12345,"COLLECTION_ID":12345,"COLLECTION_TITLE":"Season 0","PRODUCT_ID":12345,"VIDEO_TITLE":"lorem","CANONICAL_COLLECTION":{"id":12345,"name":"Season 0","href":"https://www.dropout.tv/season-0-0","parent":{"id":12345,"name":"&#39;&#38;&#59;","type":"series"}}}})";
        out.emplace_back("Html Character Code Episode Series Name Parsing", test_function, html_character_test, html_character_test_solution);

        return {"Series Name Parsing", out};
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

        return {"Embedded URL Parsing", out};
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

        return {"Config URL Parsing", out};
    }
}

std::vector<dropout_dl::tests> test_episode() {

    std::vector<dropout_dl::tests> testss;

    // testss.push_back(dropout_dl::test_episode_name_parsing());


    // testss.push_back(dropout_dl::test_episode_series_name_parsing());


    testss.push_back(dropout_dl::test_episode_embedded_url_parsing());


    testss.push_back(dropout_dl::test_episode_config_url_parsing());


    return testss;
}
