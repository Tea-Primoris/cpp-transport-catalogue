#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

namespace readers {
    class StatReader : Reader {
        using Reader::Reader;

    public:
        void ReadInput();

    private:
        void GetBusStats(std::string &args);

        void GetStopStats(std::string &args);
    };
}