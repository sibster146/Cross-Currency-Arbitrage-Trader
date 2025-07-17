# 💱 Cross-Currency Arbitrage Trader

This is a real-time cross-currency arbitrage trading bot that identifies and simulates profitable trade loops between currency pairs using Coinbase’s brokerage API. The project is implemented in C++ with a Python WebSocket data feed.

---

## ⚙️ How It Works

### 🔁 Arbitrage Concept

The bot scans the conversion rates between various cryptocurrencies and fiat pairs (e.g., USD → BTC → ETH → USD) to identify arbitrage loops that would yield a profit after accounting for bid-ask spreads and tick sizes.

---

## 🧩 Project Modules

### C++ Core

#### `main.cpp`
- Entry point of the program.
- Expects a command-line argument for `duration` (how long to run the WebSocket feed).

#### `test_Arbitrager.hpp / test_Arbitrager.cpp`
- Core logic for:
  - Product discovery and filtering.
  - Real-time order book updates via named pipe.
  - Currency graph construction.
  - Arbitrage pathfinding using DFS.
  - Trade simulation and profitability analysis.

#### `CoinbaseRequester.hpp / CoinbaseRequester.cpp`
- Handles authenticated interaction with Coinbase.
- Functions:
  - `getProducts()`
  - `getBBO()`
  - `postMarketOrder()`
  - `postLimitOrder()`
  - `getAccountInfo()`, `getOrders()`

#### `BaseRequester.hpp / BaseRequester.cpp`
- Abstract base for making HTTP requests using `libcurl`.
- Also runs a Python script (`python_jwt_gen.py`) to generate JWT tokens for authentication.

---

### Python Support

#### `test_websocket.py`
- Connects to Coinbase WebSocket API.
- Streams best bid/ask quotes for selected symbols.
- Writes the real-time data to a FIFO (`/tmp/datapipeline`) for the C++ program to consume.

#### `python_jwt_gen.py`
- Accepts a request method and path to generate a valid JWT token for Coinbase's private API access.

---

## 🔄 Workflow Overview

1. **Startup:**
   - Run `./main <duration>` in the terminal.
   - The C++ program calls `getAllProducts()` and starts the WebSocket feed using `test_websocket.py`.

2. **Streaming:**
   - The Python script receives bid/ask updates and writes JSON lines to `/tmp/datapipeline`.

3. **Parsing and Graph Update:**
   - C++ reads from the pipe.
   - Builds a directed graph of conversion rates between currencies.

4. **Arbitrage Detection:**
   - For each valid graph, DFS is used to find profitable currency cycles starting from USD.
   - Calculates the maximum viable trade size without exceeding available liquidity.

5. **Output:**
   - Prints the conversion path, trade directions (buy/sell), quantities, and profitability.

---

## 🚀 How to Run

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/Cross-Currency-Arbitrage-Trader.git
cd Cross-Currency-Arbitrage-Trader
