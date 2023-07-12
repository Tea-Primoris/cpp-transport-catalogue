#include "stat_reader.h"

namespace readers {
    void StatReader::ReadInput() {
        const int lines_to_read = GetInt();
        for (int line_number = 0; line_number < lines_to_read; ++line_number) {
            std::string line;
            std::getline(input_, line);
            std::string command = line.substr(0, line.find_first_of(' '));
            std::string update = line.substr(command.size() + 1, line.size() - command.size());

            using namespace std::string_literals;
            if (command == "Bus") {
                GetBusStats(update);
            } else if (command == "Stop") {
                GetStopStats(update);
            }
        }
    }

    void StatReader::GetBusStats(std::string &args) {
        using namespace std::string_literals;
        if (!catalogue_.HasBus(args)) {
            output_ << "Bus "s << args << ": not found"s << std::endl;
            return;
        }

        auto bus_info = catalogue_.GetBusInfo(args);
        output_ << "Bus "s << args << ": "s << bus_info.stops_on_route << " stops on route, "s
                << bus_info.unique_stops << " unique stops, "s
                << bus_info.route_length << " route length, "s
                << bus_info.curvature << " curvature"s
                << std::endl;
    }

    void StatReader::GetStopStats(std::string &args) {
        using namespace std::string_literals;
        if (!catalogue_.HasStop(args)) {
            output_ << "Stop "s << args << ": not found"s << std::endl;
            return;
        }

        const transport::Stop &stop = catalogue_.GetStop(args);

        if (stop.GetBusses().empty()) {
            output_ << "Stop "s << args << ": no buses"s << std::endl;
            return;
        }

        output_ << "Stop "s << args << ": buses"s;
        for (const transport::Bus *bus: stop.GetBusses()) {
            output_ << " "s << bus->GetNumber();
        }
        output_ << std::endl;
    }
}

