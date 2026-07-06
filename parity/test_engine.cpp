/*
 * بسم الله الرحمن الرحيم
 */

#include <iostream>
#include <cmath>
#include <string>
#include "engine.hpp"

int testsRun = 0;
int testsFailed = 0;

void check(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        std::cout << "  [PASS] " << testName << "\n";
    } else {
        std::cout << "  [FAIL] " << testName << "\n";
        testsFailed++;
    }
}

bool nearlyEqual(double a, double b, double tolerance = 0.0001) {
    return std::abs(a - b) < tolerance;
}

void testValidation() {
    std::cout << "isValidCoin / isValidCurrency:\n";
    check(isValidCoin("BTC") == true, "BTC is a valid coin");
    check(isValidCoin("banana") == false, "banana is not a valid coin");
    check(isValidCoin("") == false, "empty string is not a valid coin");
    check(isValidCurrency("GHS") == true, "GHS is a valid currency");
    check(isValidCurrency("XYZ") == false, "XYZ is not a valid currency");
}

void testParseDirection() {
    std::cout << "parseDirection:\n";
    check(parseDirection(1) == Direction::FiatToCoin, "1 parses to FiatToCoin");
    check(parseDirection(2) == Direction::CoinToFiat, "2 parses to CoinToFiat");

    bool threwOnInvalid = false;
    try {
        parseDirection(99);
    } catch (const std::invalid_argument&) {
        threwOnInvalid = true;
    }
    check(threwOnInvalid, "invalid direction value throws");
}

void testComputeConversion() {
    std::cout << "computeConversion:\n";

    ConversionResult r1 = computeConversion(60000.0, 11.5, 690000.0, Direction::FiatToCoin);
    check(nearlyEqual(r1.result, 1.0), "690,000 GHS converts to 1 BTC");
    check(nearlyEqual(r1.coinPriceInFiat, 690000.0), "coin price in fiat calculated correctly");

    ConversionResult r2 = computeConversion(60000.0, 11.5, 2.0, Direction::CoinToFiat);
    check(nearlyEqual(r2.result, 1380000.0), "2 BTC converts to 1,380,000 GHS");

    ConversionResult r3 = computeConversion(60000.0, 1.0, 100.0, Direction::CoinToFiat);
    check(nearlyEqual(r3.resultInUsd, 6000000.0), "resultInUsd calculated correctly for coin->fiat");

    bool threwOnNegative = false;
    try {
        computeConversion(60000.0, 11.5, -50.0, Direction::FiatToCoin);
    } catch (const std::invalid_argument&) {
        threwOnNegative = true;
    }
    check(threwOnNegative, "negative amount throws");

    ConversionResult r4 = computeConversion(60000.0, 11.5, 0.0, Direction::FiatToCoin);
    check(nearlyEqual(r4.result, 0.0), "zero amount returns zero result without throwing");
}

int main() {
    std::cout << "Running engine unit tests...\n\n";

    testValidation();
    testParseDirection();
    testComputeConversion();

    std::cout << "\n" << testsRun << " tests run, " << testsFailed << " failed.\n";

    return testsFailed == 0 ? 0 : 1;
}