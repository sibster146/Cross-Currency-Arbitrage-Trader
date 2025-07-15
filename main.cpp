#include "test_Arbitrager.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./main <duration>" << std::endl;
        return 1;
    }

    std::string duration = argv[1];  // Get the duration argument

    Arbitrager arbitrager;
    arbitrager.run_arbitrage(duration);

    return 0;
}
