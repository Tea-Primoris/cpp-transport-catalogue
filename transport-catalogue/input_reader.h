#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <map>
#include <algorithm>

#include "transport_catalogue.h"

namespace input_reader {
    class InputHandler {
    public:
        InputHandler() = delete;

        explicit InputHandler(transport::Catalogue &catalogue) : catalogue_(catalogue) {}

        void read_input_commands();

        void read_output_commands();

    private:
        using CommandContainer = std::unordered_map<std::string, std::deque<std::string>>;

        transport::Catalogue &catalogue_;
        std::istream &input_ = std::cin;
        std::ostream &output_ = std::cout;
        CommandContainer inputs_;
        CommandContainer outputs_;

        int get_int();

        void read_input(CommandContainer &container);

        void process_inputs();

        void read_output_input();
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