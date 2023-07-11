#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <map>
#include <algorithm>

#include "transport_catalogue.h"

namespace readers {

    class Reader {
    public:
        Reader() = delete;

        explicit Reader(transport::Catalogue &catalogue) : catalogue_(catalogue) {}

    protected:
        transport::Catalogue &catalogue_;
        std::istream &input_ = std::cin;
        std::ostream &output_ = std::cout;

        int GetInt();
    };

    class InputReader : Reader {
        using Reader::Reader;

    public:
        void ReadInput();

    private:
        using CommandContainer = std::unordered_map<std::string, std::deque<std::string>>;
        CommandContainer inputs_;

        void ProcessInputs();

        void ProcessStops();

        void ProcessBuses();

        void ProcessDistances();
    };

    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static inline void trim(std::string &s) {
        rtrim(s);
        ltrim(s);
    }
}