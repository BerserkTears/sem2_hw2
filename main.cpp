#include <cstdio>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <unistd.h>
#include <atomic>
#include <thread>
#include <vector>

#define RED "\x1b[31m"
#define GREEN "\033[0;32m"
#define DEFAULT "\033[0m"

using json = nlohmann::json;

class CurrencyChecker {
private:
    static size_t writeCallback(char *data, size_t size, size_t nitems, std::string *out) {
        if (out == nullptr) return 0;
        out->append(data, size * nitems);
        return size * nitems;
    }

    int loopTime;
    json exchangeRates;
    json previousRates;
public:
    CurrencyChecker() {
        GetRates();
        loopTime = 10;
        previousRates = exchangeRates["Valute"];
        for (json &valute: previousRates) {
            json temp = valute["Value"];
//            std::cout << valute["Value"];
            valute["Value"] = std::vector<json> ();
            valute["Value"].push_back(temp);
            valute["Sum"] = 0;
            valute["Amount"] = 0;
        }
    };

    ~CurrencyChecker() = default;

    void SetLoopTime(int newLoopTime) {
        if (newLoopTime < 1) {
            std::cout << RED << "LoopTime must equals 1 or be more" << DEFAULT;
            exit(2);
        } else {
            loopTime = newLoopTime;
        }
    }

    void GetRates() {
        CURL *curl = curl_easy_init();
        std::string response;
        if (curl) {
            CURLcode res;
            curl_easy_setopt(curl, CURLOPT_URL, "https://www.cbr-xml-daily.ru/daily_json.js");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                exit(1);
            }
            curl_easy_cleanup(curl);
        }
        exchangeRates = json::parse(response);
    }

    void PrintRates() {
        std::cout << std::fixed;
        std::cout.precision(4);
        std::cout << "Code\tExchange rate\tNominal\t\t Name" << std::endl;
        for (const json &valute: exchangeRates["Valute"]) {
            std::cout << DEFAULT << (std::string) valute["CharCode"] << "\t\t"
                      << (((double) valute["Value"] < (double) valute["Previous"]) ? GREEN : RED)
                      << (double) valute["Value"]
                      << (((double) valute["Value"] < (double) valute["Previous"]) ? " ▼" : " ▲") << " " << "\t\t"
                      << DEFAULT << (int) valute["Nominal"] << "\t\t"
                      << (std::string) valute["Name"]
                      << std::endl;
        }
        std::cout << "Date: " << (std::string) exchangeRates["Date"] << std::endl;
        std::cout << "Timestamp: " << (std::string) exchangeRates["Timestamp"] << std::endl;
    }

    void SaveRates() {
        for (const json &valute: exchangeRates["Valute"]) {
            previousRates[(std::string) valute["CharCode"]]["Value"].push_back(valute["Value"]);
            previousRates[(std::string) valute["CharCode"]]["Amount"] = (double)previousRates[(std::string) valute["CharCode"]]["Amount"] + 1;
            previousRates[(std::string) valute["CharCode"]]["Sum"] = (double)previousRates[(std::string) valute["CharCode"]]["Sum"] + (double)valute["Value"];
        }
    };

    void CalcAverages() {
        for (json &valute: previousRates) {
            valute["Average"] = (float)valute["Sum"] / (float)valute["Amount"];
//            std::cout<<valute["Average"] << std::endl;
        }
    }

    void CalcMedians() {
        for (json &valute: previousRates){
            std::sort(valute["Value"].begin(), valute["Value"].end());
            valute["Median"] = valute["Value"][(int)valute["Amount"]/2];
        }
    }

    void PrintAnalytics(){
        std::cout << std::fixed;
        std::cout.precision(4);
        std::cout << "Code\tMedian\t\t\tAverage\t\t Name" << std::endl;
        for (json &valute: previousRates) {
            std::cout << DEFAULT << (std::string) valute["CharCode"] << "\t\t"
                      << (double) valute["Median"] << "\t\t\t" <<  (double) valute["Average"] << "\t\t"
                      << (std::string) valute["Name"]
                      << std::endl;
        }
    }

    void Loop() {
        std::atomic<bool> run;
        run = true;
        std::thread stopper([&]() {
            std::cin.ignore();
            run = false;
        });
        while (run) {
            GetRates();
            SaveRates();
            PrintRates();
            int i = 0;
            while (run && i < loopTime) {
                sleep(1);
                i++;
            }
        }
        stopper.join();
        CalcMedians();
        CalcAverages();
        PrintAnalytics();
    }
};

int main() {
    CurrencyChecker lab2;
    lab2.Loop();
    return 0;
}