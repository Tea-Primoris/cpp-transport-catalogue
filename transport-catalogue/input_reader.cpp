#include "input_reader.h"

namespace input_reader {
    int InputHandler::get_int() {
        std::string line;
        std::getline(input_, line);
        return std::stoi(line);
    }

    void InputHandler::read_input_commands() {
        read_input(inputs_);
        process_inputs();
    }

    void InputHandler::read_output_commands() {
        read_output_input();
    }

    void InputHandler::read_input(CommandContainer &container) {
        const int lines_to_read = get_int();
        for (int line_number = 0; line_number < lines_to_read; ++line_number) {
            std::string line;
            std::getline(input_, line);
            std::string command = line.substr(0, line.find_first_of(' '));
            std::string update = line.substr(command.size() + 1, line.size() - command.size());
            if (command == "Stop") {
                size_t distances_start = 0;
                for (int tick = 0; tick < 2; ++tick) {
                    distances_start = update.find_first_of(',', distances_start + 1);
                    if (distances_start == std::string::npos) {
                        break;
                    }
                }
                if (distances_start != std::string::npos) {
                    std::string distances = update.substr(0, update.find_first_of(':')) + ", " + update.substr(distances_start + 2);
                    container["Distances"].push_back(distances);
                }
            }
            container[command].push_back(std::move(update));
        }
    }

    void InputHandler::read_output_input() {
        const int lines_to_read = get_int();
        for (int line_number = 0; line_number < lines_to_read; ++line_number) {
            std::string line;
            std::getline(input_, line);
            std::string command = line.substr(0, line.find_first_of(' '));
            std::string update = line.substr(command.size() + 1, line.size() - command.size());
            using namespace std::string_literals;
            if (command == "Bus") {
                const transport::Bus *bus;
                try {
                    bus = &catalogue_.get_bus(update);
                }
                catch (std::out_of_range &) {
                    output_ << "Bus "s << update << ": not found"s << std::endl;
                    continue;
                }

                auto bus_route_distance = catalogue_.get_bus_route_distance(update);
                output_ << "Bus "s << update << ": "s << bus->get_stops_count() << " stops on route, "s
                        << bus->count_unique_stops() << " unique stops, "s
                        << bus_route_distance
                        << " route length, "s
                        << bus_route_distance / catalogue_.get_bus_route_geo_distance(update)
                        << " curvature"s
                        << std::endl;

            } else if (command == "Stop") {
                const transport::Stop *stop;
                try {
                    stop = &catalogue_.get_stop(update);
                }
                catch (std::out_of_range &) {
                    output_ << "Stop "s << update << ": not found"s << std::endl;
                    continue;
                }
                if (stop->get_buses().empty()) {
                    output_ << "Stop "s << update << ": no buses"s << std::endl;
                    continue;
                }
                output_ << "Stop "s << update << ": buses"s;
                for (const transport::Bus *bus: stop->get_buses()) {
                    output_ << " "s << bus->get_number();
                }
                output_ << std::endl;
            }
        }
    }

    void InputHandler::process_inputs() {
        for (const std::string &line: inputs_["Stop"]) {
            std::string stop_name = line.substr(0, line.find_first_of(':'));
            const size_t latitude_end = line.find_first_of(',', stop_name.size() + 1);
            const size_t longitude_end = line.find_first_of(',', latitude_end + 1);
            const double latitude = stod(line.substr(stop_name.size() + 2, latitude_end - stop_name.size() - 2));
            const double longitude = stod(line.substr(latitude_end + 2, longitude_end - latitude_end + 2));
            catalogue_.add_stop(std::move(stop_name), latitude, longitude);

        }

        for (const std::string &line: inputs_["Bus"]) {
            std::string bus_name = line.substr(0, line.find_first_of(':'));

            char delim = line[line.find_first_of(">-", bus_name.size())];
            bool is_circle_route = delim == '>';

            std::vector<std::string> stops;
            for (size_t stop_name_start = bus_name.size() + 2; stop_name_start != std::string::npos;) {
                stop_name_start = line.find_first_not_of(' ', stop_name_start);
                size_t stop_name_end = line.find_first_of(delim, stop_name_start);
                std::string stop_name = line.substr(stop_name_start, stop_name_end - stop_name_start);
                trim(stop_name);
                stops.push_back(std::move(stop_name));
                stop_name_start = stop_name_end == std::string::npos ? std::string::npos : stop_name_end + 1;
            }

            catalogue_.add_bus(bus_name, stops, is_circle_route);
        }

        for (const std::string &line: inputs_["Distances"]) {
            const size_t from_stop_name_length = line.find_first_of(',');
            transport::Stop &from_stop = catalogue_.get_stop(line.substr(0, from_stop_name_length));
            for (size_t distance_start = from_stop_name_length + 1; distance_start != std::string::npos;) {
                size_t distance_end = line.find_first_of('m', distance_start);
                int distance = std::stoi(line.substr(distance_start, distance_end - distance_start));

                size_t to_stop_start = distance_end + 5;
                size_t to_stop_end = line.find_first_of(',', to_stop_start);
                transport::Stop &to_stop = catalogue_.get_stop(line.substr(to_stop_start, to_stop_end - to_stop_start));

                std::pair<transport::Stop *, transport::Stop *> stop_pair;
                stop_pair.first = &from_stop;
                stop_pair.second = &to_stop;
                catalogue_.add_distance(stop_pair, distance);

                distance_start = to_stop_end != std::string::npos ? to_stop_end + 1 : std::string::npos;
            }
        }
    }
}