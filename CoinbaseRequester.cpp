#include "CoinbaseRequester.hpp"
// #include "json.hpp"
// #include <iostream>
// #include <vector>
// #include <unordered_map>
// #include <cmath>


// Constructor
CoinbaseRequester::CoinbaseRequester() {}

// Function to execute the Python script and capture its output
std::string CoinbaseRequester::getJWT(const std::string& request_method, const std::string& api_argument) {

    std::string file_name = "python_jwt_gen.py";
    std::string argument = request_method + " " + api_argument;

    return invokePythonFunction(file_name, argument);
}

std::string CoinbaseRequester::requestHelper(const std::string& base_url, const std::string& api_url, const std::string& data_args, const std::string& request_method, const nlohmann::json& payload = nlohmann::json()){
    if ((api_to_token.find(api_url)==api_to_token.end()) || (api_to_token[api_url].first + std::chrono::minutes(3) > std::chrono::system_clock::now())) {
        std::string jwt = getJWT(request_method, api_url);
        api_to_token[api_url] = {std::chrono::system_clock::now(), jwt};
    }

    const std::string& jwt = api_to_token[api_url].second;

    if (request_method == "GET"){
        return getRequest(base_url, api_url, data_args, {"Authorization: Bearer " + jwt});
    }
    else {
        std::string body = payload.dump();
        return postRequest(base_url, api_url, data_args, body, {"Authorization: Bearer " + jwt});
    }
}



// Function to make a curl request
nlohmann::json CoinbaseRequester::getBBO(const std::vector<std::string>& symbols) {

    const std::string request_method = "GET";
    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "best_bid_ask";
    std::string data_args = "?";
    
    for (int i = 0; i < symbols.size(); i++){

        data_args = data_args + "product_ids="+symbols[i];
        if (i != symbols.size()-1){
            data_args = data_args+"&";
        }
    }

    const std::string response = requestHelper(base_url, api_url, data_args, request_method);

    if (response == "Unauthorized"){
        std::cerr << "BBO request is unathorized" << std::endl;
        throw std::runtime_error("BBO request is unathorized");
    }

    try {
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        const nlohmann::json pricebook = response_json_obj["pricebooks"];
        return pricebook;
    } catch (...){
        std::cerr << "Error in parsing BBO response: " << response << std::endl;
        throw std::runtime_error("Error in parsing BBO response: " + response);
    }
}





nlohmann::json CoinbaseRequester::postMarketOrder(const std::string& client_order_id, const std::string& product_id, const std::string& side, const std::string& amount){
    // std::unordered_map<std::string, std::string> side_map = {{"SELL","base_size"},{"BUY","quote_size"}};

    // nlohmann::json product_id_pricebook = getBBO({product_id});
    // nlohmann::json price = product_id_pricebook[0];
    // price["asks"][0]["price"];


    const nlohmann::json payload = {
        {"client_order_id", client_order_id},
        {"product_id", product_id},
        {"side", side},
        {"order_configuration", {
            {"market_market_ioc", {
                {"base_size", amount}
            }}
        }}
    };

    // Convert JSON to string for POST request
    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "orders";
    const std::string request_method = "POST";
    const std::string data = "";

    const std::string response = requestHelper(base_url, api_url, data, request_method, payload);

    if (response == "Unauthorized"){
        std::cerr << "postMarketOrder request is unathorized" << std::endl;
        throw std::runtime_error("postMarketOrder is Unauthorized");
    }

    try {
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        return response_json_obj;
    } catch (...){
        std::cerr << "Error in parsing postMarketOrder response: " << response << std::endl;
        throw std::runtime_error("Error in parsing BBO response: " + response);
    }
    /*
    {"success":true,"success_response":{"order_id":"c05a3d0f-0631-4e5f-9585-f335d3bfacae","product_id":"BTC-USD","side":"BUY","client_order_id":"4","attached_order_id":""},"order_configuration":{"market_market_ioc":{"quote_size":"5","rfq_enabled":false,"rfq_disabled":false}}}
    */
}

nlohmann::json CoinbaseRequester::postLimitOrder(const std::string& client_order_id, const std::string& product_id, const std::string& side, const std::string& amount, const std::string& limit_price){
    double base_size;
    if (side == "BUY")
        base_size = std::stod(amount)/std::stod(limit_price);
    else{
        base_size = std::stod(amount);
    }

    const nlohmann::json payload = {
        {"client_order_id", client_order_id},
        {"product_id", product_id},
        {"side", side},
        {"order_configuration", {
            {"limit_limit_gtc", {
                {"limit_price",limit_price},
             //    {"base_size", std::to_string(base_size)},
                {"base_size", amount},
                {"post_only",true}
            }}
        }}
    };

    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "orders";
    const std::string request_method = "POST";
    const std::string data = "";

    const std::string response = requestHelper(base_url, api_url, data, request_method, payload);

    if (response == "Unauthorized"){
        std::cerr << "postMarketOrder request is unathorized" << std::endl;
        throw std::runtime_error("postMarketOrder is Unauthorized");
    }

    try {
        std::cout << response <<std::endl;
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        return response_json_obj;
    } catch (...){
        std::cerr << "Error in parsing postMarketOrder response: " << response << std::endl;
        throw std::runtime_error("Error in parsing BBO response: " + response);
    }
}

nlohmann::json CoinbaseRequester::getOrders(){
    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "orders/historical/batch";
    const std::string data = "?order_status=FILLED";
    const std::string request_method = "GET";

    const std::string response = requestHelper(base_url, api_url, data, request_method);
    if (response == "Unauthorized"){
        std::cerr << "getOrders request is unathorized" << std::endl;
        throw std::runtime_error("getOrders is Unauthorized");
    }

    try {
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        const nlohmann::json orders = response_json_obj["orders"];
        return orders;
    } catch (...){
        std::cerr << "Error in parsing getOrders: " << response << std::endl;
        throw std::runtime_error("Parsing error in getOrders: " + response);
    }
}

nlohmann::json CoinbaseRequester::getProducts(){

    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "products";
    const std::string data = "";
    const std::string request_method = "GET";

    const std::string response = requestHelper(base_url, api_url, data, request_method);
    if (response == "Unauthorized"){
        std::cerr << "getProducts request is unathorized" << std::endl;
        throw std::runtime_error("getProducts is Unauthorized");
    }

    try {
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        const nlohmann::json products = response_json_obj["products"];
        return products;
    } catch (...) {
        std::cerr << "Error in parsing getProducts: " << response << std::endl;
        throw std::runtime_error("Parsing error in getProducts: " + response);
    }
}

nlohmann::json CoinbaseRequester::getAccountInfo(){
    const std::string base_url = "https://api.coinbase.com/api/v3/brokerage/";
    const std::string api_url = "accounts";
    const std::string request_method = "GET";
    const std::string data = "";
    const std::string response = requestHelper(base_url, api_url,data, request_method);

    try {
        const nlohmann::json response_json_obj = nlohmann::json::parse(response);
        const nlohmann::json accounts = response_json_obj["accounts"];
        return accounts;
    }
    catch (...) {
        std::cerr << "Error in parsing Accounts: " << response << std::endl;
        throw std::runtime_error("Error in parsing Accounts: " + response);
    }


}








