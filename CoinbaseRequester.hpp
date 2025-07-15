#ifndef COINBASE_REQUESTER_HPP
#define COINBASE_REQUESTER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <curl/curl.h>
#include <memory>
#include <stdexcept>
#include <array>
#include "BaseRequester.hpp"
#include "json.hpp"
#include <iostream>
#include <cmath>

class CoinbaseRequester : public BaseRequester{
    std::unordered_map<std::string, std::pair<std::chrono::system_clock::time_point, std::string>> api_to_token;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> conversionRateMap;

    std::unordered_map<std::string,int> decimal_places = {
        {"USD", 1e2},
        {"ETH", 1e5},
        {"USDT", 1e4},
        
    };

public:
    CoinbaseRequester();

    // int getDecimalValue(const std::string& currency, int default_value){
    //     if (!decimal_places.contains(currency)){
    //         return default_value;
    //     }
    //     return decimal_places[currency];
    // }

    std::string requestHelper(const std::string& base_url, const std::string& api_url, const std::string& data, const std::string& request_method, const nlohmann::json& payload);

    // Function to execute the Python script and capture its output
    std::string getJWT(const std::string& request_method, const std::string& api_argument);

    // Function to make a curl request
    nlohmann::json getBBO(const std::vector<std::string>& symbols);

    // std::string postOrderHelper(nlohmann::json& payload);

    nlohmann::json postMarketOrder(const std::string& client_order_id, const std::string& product_id, const std::string& side, const std::string& quote_size);

    nlohmann::json postLimitOrder(const std::string& client_order_id, const std::string& product_id, const std::string& side, const std::string& amount, const std::string& limit_price);

    nlohmann::json getOrders();

    nlohmann::json getProducts();

    nlohmann::json getAccountInfo();

};

#endif // COINBASE_REQUESTER_HPP

