#include "test_Arbitrager.hpp"


Arbitrager::Arbitrager() = default;

std::pair<std::string, int> Arbitrager::getAllProducts(){

    nlohmann::json products = coinbase_requester.getProducts();

    std::string str_display_names;
    std::unordered_set<std::string> total_num_symbols;

    for (int i = 0; i < products.size(); i++){
        nlohmann::json product = products[i];
        std::string display_name = product["display_name"];
        size_t slash_pos = display_name.find("-");
        if (slash_pos==std::string::npos){
            continue;
        }
        std::string to_currency = display_name.substr(0, slash_pos); // BTC-USD ; BTC is to, USD is from
        std::string from_currency = display_name.substr(slash_pos+1);
        if ((banned_currencies.find(to_currency)!= banned_currencies.end())||(banned_currencies.find(from_currency) != banned_currencies.end())){
            continue;
        }

        str_display_names += display_name;
        if (i < products.size() - 1){
            str_display_names += ",";
        }

        std::string base_increment_str = product["base_increment"];
        std::string quote_increment_str = product["quote_increment"];
        long double base_increment = std::stold(base_increment_str);
        long double quote_increment = std::stold(quote_increment_str);
        symbol_increment[display_name] = {base_increment, quote_increment};

        total_num_symbols.insert(to_currency);
        total_num_symbols.insert(from_currency);
    }

    return std::make_pair(str_display_names, total_num_symbols.size());
}

void Arbitrager::update_top_orderbook(std::string& line, std::unordered_set<std::string>& processed_symbols) {

    try {

        if (line.empty() || line[0] != '{') {
            std::cerr << "Skipping non-JSON line: " << line << std::endl;
            return;
        }

        nlohmann::json line_json = nlohmann::json::parse(line);
        nlohmann::json tickers = line_json["events"][0]["tickers"];
        nlohmann::json ticker = tickers[0];

        std::string display_name = ticker["product_id"];

        size_t slash_pos = display_name.find("-");
        if (slash_pos==std::string::npos){
            return;
        }
        std::string to_currency = display_name.substr(0, slash_pos); // BTC-USD ; BTC is to, USD is from
        std::string from_currency = display_name.substr(slash_pos+1);
        if ((banned_currencies.find(to_currency)!= banned_currencies.end())||(banned_currencies.find(from_currency) != banned_currencies.end())){
            return;
        }

        std::string str_best_bid_price = ticker["best_bid"];
        std::string str_best_ask_price = ticker["best_ask"];

        std::string str_best_bid_quantity = ticker["best_bid_quantity"];
        std::string str_best_ask_quantity = ticker["best_ask_quantity"];

        long double best_bid_price = std::stold(str_best_bid_price);
        long double best_ask_price = std::stold(str_best_ask_price);
        long double best_bid_quantity = std::stold(str_best_bid_quantity);
        long double best_ask_quantity = std::stold(str_best_ask_quantity);

        auto [base_increment, quote_increment] = symbol_increment[display_name];

        //{
        // std::lock_guard<std::mutex> lock(conversionRateMutexMap[display_name]);
        conversionRateMap[from_currency][to_currency] = {1 / best_ask_price, best_ask_quantity, Side::buy, quote_increment};
        conversionRateMap[to_currency][from_currency] = {best_bid_price, best_bid_quantity, Side::sell, base_increment};
        processed_symbols.insert(from_currency);
        processed_symbols.insert(to_currency);
        //}

        } 
    catch (const nlohmann::json::exception& e) {
            //std::cerr << "JSON error: " << e.what() << "\nOffending line: " << line << std::endl;
        }
}



std::vector<std::string> Arbitrager::find_arbitrage(const std::string& start_currency){
    std::unordered_map<std::string, long double> best_conversion_yet;
    std::vector<std::string> path_vec;
    std::unordered_set<std::string> path_set;
    std::vector<std::string> final_path;

  //  while (1){
    find_arbitrage_helper(
    start_currency, 100.0 , best_conversion_yet, 
    path_vec, path_set, final_path, start_currency
    );

    return final_path;

}


void Arbitrager::find_arbitrage_helper(const std::string& curr_currency, long double curr_amount, std::unordered_map<std::string,long double>& best_conversion_yet,std::vector<std::string>& path_vec,std::unordered_set<std::string>& path_set,
    std::vector<std::string>& final_path, const std::string& start_currency) {
    if (curr_currency == start_currency && path_vec.size() > 0) {
        best_conversion_yet[curr_currency] = curr_amount;
        path_vec.push_back(curr_currency);
        final_path = path_vec;
        path_vec.pop_back();
        return;
    }

    path_vec.push_back(curr_currency);
    path_set.insert(curr_currency);
    best_conversion_yet[curr_currency] = curr_amount;

    for (auto to_it = conversionRateMap[curr_currency].begin(); to_it != conversionRateMap[curr_currency].end(); to_it++) {
        if (to_it->first != start_currency && (path_set.find(to_it->first)!= path_set.end())) {
            continue;
        }

        std::string next_currency = to_it->first;
        double next_amount = curr_amount * std::get<0>(to_it->second);
        if ((best_conversion_yet.find(next_currency)!=best_conversion_yet.end()) && best_conversion_yet[next_currency] >= next_amount) continue;
        
        find_arbitrage_helper(next_currency, next_amount, best_conversion_yet, path_vec, path_set, final_path, start_currency);
    }

    path_vec.pop_back();
    path_set.erase(curr_currency);

}

long double Arbitrager::getMaxStartSize(const std::vector<std::string>& path){

    long double max_quantity = std::numeric_limits<long double>::infinity();
    for (int i = path.size()-2; i >= 0; i--){
        std::string from = path[i];
        std::string to = path[i+1];
        auto [price, size, side, increment] = conversionRateMap[from][to];
        if (side == Side::sell){
            max_quantity = std::min(price*size, max_quantity);
            max_quantity = max_quantity/price;
        } else {
            max_quantity = std::min(max_quantity, size);
            max_quantity = max_quantity / price;
        }
    }

    return max_quantity;
}


void Arbitrager::printPath(double curr_balance, const std::vector<std::string>& path){
    curr_balance = std::round(curr_balance*1e2) / 1e2;
    double intial_balance = curr_balance;
    // curr_balance = std::min(curr_balance, 50.00);
    // std::cout << "Initial Balance: " << curr_balance << std::endl;
    std::vector<std::tuple<double, std::string, std::string>> res_list;

    for (int i = 0; i < path.size()-1; i++){
        std::string from = path[i];
        std::string to = path[i+1];
        auto [price, size, side, increment] = conversionRateMap[from][to];
        curr_balance = (std::floor(curr_balance / increment) * increment);
        std::string product_id;
        if (side == Side::sell){
            product_id = from+"-"+to;
            res_list.push_back({curr_balance, product_id, "sell"});
            // std::cout << "  " << from <<"-"<<to<<" "<<price<<" "<<size<< " (SELL) Curr Balance: " << curr_balance << std::endl;
        } else {
            product_id = to+"-"+from;
            res_list.push_back({curr_balance, product_id, "buy"});
            // std::cout << "  " << to <<"-"<<from<<" "<<(1/price)<<" "<<size<<" (BUY) Curr Balance: " << curr_balance << std::endl;
        }
        curr_balance = curr_balance * price;


    }
    
    // std::cout << "Final Balance: " << curr_balance << std::endl;
    for (int i = 0; i < res_list.size(); i++){
        auto [num, curr, side] = res_list[i];
        std::cout << num << " " << curr << " " << side << "-->";
    }

    std::cout << curr_balance << " USD; ";
    if (intial_balance > curr_balance){
        std::cout << "NOT PROFITABLE BECAUSE OF TICK SIZE ROUNDING" << std::endl;
    }
    else {
        std::cout << std::endl;
    }
}


void Arbitrager::run_arbitrage(std::string duration) {

    // Get all products
    std::pair<std::string,int> str_display_names_size_pair = getAllProducts();
    std::string str_display_names = str_display_names_size_pair.first;
    int total_num_symbols = str_display_names_size_pair.second;
    
    // Open the named pipe and start the websocket
    const std::string fifo_path = "/tmp/datapipeline";
    std::remove(fifo_path.c_str());
    if (mkfifo(fifo_path.c_str(), 0666) != 0) {
        std::cerr << "Failed to create FIFO at " << fifo_path << std::endl;
        return;
    }
    std::string python_exec = "python3 test_websocket.py " + str_display_names + " " + duration + " &";
    std::system(python_exec.c_str());
    std::ifstream pipe(fifo_path);
    if (!pipe.is_open()) {
        std::cerr << "Failed to open named pipe." << std::endl;
        return;
    }
    std::cout << "Opened pipeline. Listening for data...\n";

    // Read from the named pipe and update the order book
    std::unordered_set<std::string> processed_symbols;
    std::string line;
    while (std::getline(pipe, line)) {
        update_top_orderbook(line, processed_symbols);
        if (processed_symbols.size() >= total_num_symbols-10) {
            std::vector<std::string> currency_path = find_arbitrage("USD");

            long double initial_amount = getMaxStartSize(currency_path);
            printPath(initial_amount, currency_path);

        } else {
            std::cout << processed_symbols.size() << " / " << total_num_symbols << " symbols processed.\n";
        }
    }

    pipe.close();
    std::cout << "Exiting C++ program.\n";

}
