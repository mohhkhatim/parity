/*
 * بسم الله الرحمن الرحيم
 */

#include <iostream>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include "engine.hpp"

int main() {
    httplib::Server svr;

    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"}
    });

    svr.Get("/convert", [](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string coin = req.get_param_value("coin");
            std::string currency = req.get_param_value("currency");

            if (!req.has_param("amount") || !req.has_param("direction")) {
                throw std::invalid_argument("Missing required parameter: amount and direction are required");
            }

            double amount = std::stod(req.get_param_value("amount"));
            int direction = std::stoi(req.get_param_value("direction"));

            ConversionResult result = convert(coin, currency, amount, direction);

            nlohmann::json responseJson = {
                {"coinPriceInFiat", result.coinPriceInFiat},
                {"coinPriceUsd", result.coinPriceUsd},
                {"result", result.result},
                {"resultInUsd", result.resultInUsd}
            };

            res.set_content(responseJson.dump(), "application/json");

        } catch (const std::invalid_argument& e) {
            nlohmann::json errorJson = {{"error", e.what()}};
            res.status = 400;
            res.set_content(errorJson.dump(), "application/json");

        } catch (const std::exception& e) {
            std::cerr << "[error] /convert failed: " << e.what() << std::endl;
            nlohmann::json errorJson = {{"error", "Unable to fetch live rates right now. Please try again shortly."}};
            res.status = 502;
            res.set_content(errorJson.dump(), "application/json");
        }
    });

    std::cout << "Server running at http://localhost:8080\n";
    svr.listen("localhost", 8080);

    return 0;
}