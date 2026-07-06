/*
 * بسم الله الرحمن الرحيم
 */


#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <future>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

// ================= VALIDATION =================

const std::vector<std::string> VALID_COINS = {"BTC", "ETH", "BNB", "LTC", "USDT", "SOL", "XRP"};
const std::vector<std::string> VALID_CURRENCIES = {"GHS", "NGN", "KES", "USD"};

inline bool isValidCoin(const std::string& coin) {
    return std::find(VALID_COINS.begin(), VALID_COINS.end(), coin) != VALID_COINS.end();
}

inline bool isValidCurrency(const std::string& currency) {
    return std::find(VALID_CURRENCIES.begin(), VALID_CURRENCIES.end(), currency) != VALID_CURRENCIES.end();
}

enum class Direction {
    FiatToCoin = 1,
    CoinToFiat = 2
};

inline Direction parseDirection(int value) {
    if (value == 1) return Direction::FiatToCoin;
    if (value == 2) return Direction::CoinToFiat;
    throw std::invalid_argument("Direction must be 1 (fiat->coin) or 2 (coin->fiat)");
}

// ================= MATH =================

struct ConversionResult {
    double coinPriceInFiat;
    double coinPriceUsd;
    double result;
    double resultInUsd;
};

inline ConversionResult computeConversion(double coinPriceUsd, double usdToFiat, double amount, Direction direction) {
    if (amount < 0) {
        throw std::invalid_argument("Amount cannot be negative");
    }

    double coinPriceInFiat = coinPriceUsd * usdToFiat;

    ConversionResult res;
    res.coinPriceInFiat = coinPriceInFiat;
    res.coinPriceUsd = coinPriceUsd;

    if (direction == Direction::FiatToCoin) {
        res.result = amount / coinPriceInFiat;
        res.resultInUsd = res.result * coinPriceUsd;
    } else {
        res.result = amount * coinPriceInFiat;
        res.resultInUsd = amount * coinPriceUsd;
    }

    return res;
}

// ================= CACHE =================

struct CachedValue {
    double value;
    std::chrono::steady_clock::time_point fetchedAt;
};

inline std::mutex cacheMutex;
inline std::map<std::string, CachedValue> forexCache;
inline std::map<std::string, CachedValue> coinCache;

const std::chrono::seconds CACHE_DURATION(30);

inline bool isFresh(const std::chrono::steady_clock::time_point& fetchedAt) {
    return (std::chrono::steady_clock::now() - fetchedAt) < CACHE_DURATION;
}

// ================= NETWORK FETCHING  =================

inline double fetchUsdToFiatRate(const std::string& fiatCode) {
    if (fiatCode == "USD") return 1.0;

    cpr::Response r = cpr::Get(
        cpr::Url{"https://open.er-api.com/v6/latest/USD"},
        cpr::Timeout{5000}
    );
    if (r.status_code != 200) {
        throw std::runtime_error("forex API returned status " + std::to_string(r.status_code));
    }
    nlohmann::json data = nlohmann::json::parse(r.text);
    if (!data.contains("rates") || !data["rates"].contains(fiatCode)) {
        throw std::runtime_error("currency code not found in forex response: " + fiatCode);
    }
    return data["rates"][fiatCode];
}

inline double fetchCoinPriceUsdt(const std::string& coin) {
    if (coin == "USDT") return 1.0;

    std::string symbol = coin + "USDT";
    cpr::Response r = cpr::Get(
        cpr::Url{"https://api.binance.com/api/v3/ticker/price"},
        cpr::Parameters{{"symbol", symbol}},
        cpr::Timeout{5000}
    );

    if (r.status_code != 200) {
        throw std::runtime_error("Binance price API returned status " + std::to_string(r.status_code));
    }

    nlohmann::json data = nlohmann::json::parse(r.text);

    if (data.contains("code")) {
        throw std::runtime_error("Binance error for " + coin + ": " + data.value("msg", "unknown error"));
    }

    return std::stod(data["price"].get<std::string>());
}

inline double getUsdToFiatRateCached(const std::string& fiatCode) {
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = forexCache.find(fiatCode);
    if (it != forexCache.end() && isFresh(it->second.fetchedAt)) {
        return it->second.value;
    }

    double rate = fetchUsdToFiatRate(fiatCode);
    forexCache[fiatCode] = {rate, std::chrono::steady_clock::now()};
    return rate;
}

inline double getCoinPriceCached(const std::string& coin) {
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = coinCache.find(coin);
    if (it != coinCache.end() && isFresh(it->second.fetchedAt)) {
        return it->second.value;
    }

    double price = fetchCoinPriceUsdt(coin);
    coinCache[coin] = {price, std::chrono::steady_clock::now()};
    return price;
}

// ================= CONVERT (validation + network + math) =================

inline ConversionResult convert(const std::string& coin, const std::string& currency, double amount, int directionValue) {
    if (!isValidCoin(coin)) {
        throw std::invalid_argument("Unsupported coin: " + coin);
    }
    if (!isValidCurrency(currency)) {
        throw std::invalid_argument("Unsupported currency: " + currency);
    }
    Direction direction = parseDirection(directionValue);

    std::future<double> forexFuture = std::async(std::launch::async, getUsdToFiatRateCached, currency);
    std::future<double> coinFuture = std::async(std::launch::async, getCoinPriceCached, coin);

    double usdToFiat = forexFuture.get();
    double coinPriceUsd = coinFuture.get();

    return computeConversion(coinPriceUsd, usdToFiat, amount, direction);
}