#ifndef TEST_ARBITRAGER_HPP
#define TEST_ARBITRAGER_HPP


#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include <chrono>
#include <thread>
#include <cstdio>      // std::remove
#include <sys/stat.h>  // mkfifo
#include <unistd.h>    // access
#include "CoinbaseRequester.hpp"
#include <mutex>
#include <thread>
#include <cmath>

enum class Side {
    buy,
    sell
};

class Arbitrager {

    CoinbaseRequester coinbase_requester;
    std::unordered_set<std::string> banned_currencies = {};
    std::unordered_map<std::string, std::pair<long double, long double>> symbol_increment;  // base, quote
    std::unordered_map<std::string, std::unordered_map<std::string, std::tuple<long double,long double, Side, long double>>> conversionRateMap;
    std::unordered_map<std::string,std::mutex> conversionRateMutexMap;

    public:
    Arbitrager();

    std::pair<std::string, int> getAllProducts();

    void open_pipe_websocket(std::string& str_display_names, std::string& duration);

    void update_top_orderbook(std::string& line, std::unordered_set<std::string>& processed_symbols);

    long double getMaxStartSize(const std::vector<std::string>& path);

    void printPath(double curr_balance, const std::vector<std::string>& path);

    // int update_top_orderbook(std::string str_display_names, int total_num_symbols, std::string duration);

    void run_arbitrage(std::string duration);

    std::vector<std::string> find_arbitrage(const std::string& start_currency);

    void find_arbitrage_helper(const std::string& curr_currency, long double curr_amount, std::unordered_map<std::string,long double>& best_conversion_yet,std::vector<std::string>& path_vec,std::unordered_set<std::string>& path_set,
    std::vector<std::string>& final_path, const std::string& start_currency);

    
};



#endif

