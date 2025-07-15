#ifndef BASE_REQUESTER_HPP
#define BASE_REQUESTER_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <curl/curl.h>
#include <memory>
#include <stdexcept>
#include <array>
#include <mutex>
#include <ctime>
#include "json.hpp"
#include <iostream>
#include <chrono>



class BaseRequester {
    static std::mutex std_out_mutex;
public:
    BaseRequester();


    // Callback function to capture the response
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    std::string getRequest(const std::string& base_url, const std::string& api_url, const std::string& data, const std::vector<std::string>& auth_strs);

    std::string postRequest(const std::string& base_url, const std::string& api_url, const std::string& data, const std::string& body, const std::vector<std::string>& auth_strs);

    std::string invokePythonFunction(const std::string& python_file, const std::string& argument);


};

#endif // BASE_REQUESTER_HPP
