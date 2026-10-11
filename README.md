# Parity

A crypto and currency converter written in C++17.

- Live crypto prices from CoinGecko and forex rates from open.er-api.com
- 30-second cache and concurrent fetching
- Input validation with clear 400/502 error responses
- 14 unit tests with a hand-written test runner
- Dark web frontend in HTML, CSS and JavaScript

Built with C++17, CMake, cpr, nlohmann/json and cpp-httplib.

## Build and run (macOS, Homebrew)

    brew install cmake cpr nlohmann-json cpp-httplib openssl@3
    cd parity
    mkdir build && cd build
    cmake .. && make
    ./run_tests
    ./rate_tracker

Then open `parity/public/index.html` in a browser while the server is running.

The API is at `http://localhost:8080/convert?coin=BTC&currency=GHS&amount=1&direction=2`
(direction 1 = currency to coin, 2 = coin to currency).

Note: `CMakeLists.txt` points OpenSSL at the Apple Silicon Homebrew path (`/opt/homebrew`).
