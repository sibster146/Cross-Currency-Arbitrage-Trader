#include "BaseRequester.hpp"
// #include "json.hpp"
// #include <iostream>
// #include <chrono>

std::mutex BaseRequester::std_out_mutex;
// Constructor
BaseRequester::BaseRequester() {}

// Static callback function
size_t BaseRequester::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string BaseRequester::getRequest(const std::string& base_url, const std::string& api_url, const std::string& data, const std::vector<std::string>& auth_strs = std::vector<std::string>()){

    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        // Construct the URL
        const std::string complete_url = base_url + api_url + data;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, complete_url.c_str());

        // Set the Authorization header
        struct curl_slist* headers = nullptr;
        if (!auth_strs.empty()){
            for (auto& auth_str : auth_strs){
                headers = curl_slist_append(headers, (auth_str).c_str());
            }
            // headers = curl_slist_append(headers, ("Authorization: Bearer " + jwt).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        // Set up the callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        try {
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK) {
                std::cerr << "Get request response returned with errors: " << std::string(curl_easy_strerror(res)) << std::endl;
                throw std::runtime_error("Get request response returned with errors: " + std::string(curl_easy_strerror(res)));
                return "-1";
            }
        } catch (const std::exception& e) {
            std::cerr << "Get request failed: " << e.what() << std::endl;
            throw std::runtime_error("Get request failed: " + std::string(e.what()));
            return "-1";
        }

        // Clean up
        if (!auth_strs.empty()){
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);

        return response;

    } else {
        std::cerr << "initializing curl failed." << std::endl;
        throw std::runtime_error("Initializing curl failed.");
        return "-1";
    }
}

std::string BaseRequester::postRequest(const std::string& base_url, const std::string& api_url, const std::string& data, const std::string& body, const std::vector<std::string>& auth_strs = std::vector<std::string>()){
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        // Construct the URL
        const std::string complete_url = base_url + api_url + data;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, complete_url.c_str());
        // Specify that this is a POST request
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // Set the request body
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        // Set the Authorization header
        struct curl_slist* headers = nullptr;
        for (auto& auth_str : auth_strs){
            headers = curl_slist_append(headers, (auth_str).c_str());
        }
        // headers = curl_slist_append(headers, ("Authorization: Bearer " + jwt).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        try {
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK) {
                std::cerr << "Get request response returned with errors: " << std::string(curl_easy_strerror(res)) << std::endl;
                throw std::runtime_error("Get request response returned with errors: " + std::string(curl_easy_strerror(res)));
                return "-1";
            }
        } catch (const std::exception& e) {
            std::cerr << "Get request failed: " << e.what() << std::endl;
            throw std::runtime_error("Get request failed: " + std::string(e.what()));
            return "-1";
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return response;
    } else {
        std::cerr << "initializing curl failed." << std::endl;
        throw std::runtime_error("Initializing curl failed.");
        return "-1";
    }
}
std::string BaseRequester::invokePythonFunction(const std::string& file_name, const std::string& argument){

    std::array<char, 128> buffer;
    std::string result;
    std::string command = "python3 " + file_name + " " + argument;

    // Open a pipe to execute the Python script
    std_out_mutex.lock();
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std_out_mutex.unlock();
        std::cerr << "Failed to run Python script" << std::endl;
        std::runtime_error("Failed to run Python script");
        return "-1";
    }

    // Read the script output
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    std_out_mutex.unlock();

    // Trim any trailing whitespace or newline
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return result;
}
