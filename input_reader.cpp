#include "input_reader.h"

namespace readers {
    int Reader::GetInt() {
        std::string line;
        std::getline(input_, line);
        return std::stoi(line);
    }

    void InputReader::ReadInput() {
        const int lines_to_read = GetInt();
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
                    std::string distances =
                            update.substr(0, update.find_first_of(':')) + ", " + update.substr(distances_start + 2);
                    inputs_["Distances"].push_back(distances);
                }
            }
            inputs_[command].push_back(std::move(update));
        }

        ProcessInputs();
    }

    void InputReader::ProcessInputs() {
        ProcessStops();
        ProcessBuses();
        ProcessDistances();
    }

    void InputReader::ProcessStops() {
        for (const std::string &line: inputs_["Stop"]) {
            std::string stop_name = line.substr(0, line.find_first_of(':'));
            const size_t latitude_end = line.find_first_of(',', stop_name.size() + 1);
            const size_t longitude_end = line.find_first_of(',', latitude_end + 1);
            const double latitude = stod(line.substr(stop_name.size() + 2, latitude_end - stop_name.size() - 2));
            const double longitude = stod(line.substr(latitude_end + 2, longitude_end - latitude_end + 2));
            catalogue_.AddStop(std::move(stop_name), {latitude, longitude});
        }
    }

    void InputReader::ProcessBuses() {
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

            catalogue_.AddBus(bus_name, stops, is_circle_route);
        }
    }

    void InputReader::ProcessDistances() {
        for (const std::string &line: inputs_["Distances"]) {
            const size_t from_stop_name_length = line.find_first_of(',');
            transport::Stop &from_stop = catalogue_.GetStop(line.substr(0, from_stop_name_length));
            for (size_t distance_start = from_stop_name_length + 1; distance_start != std::string::npos;) {
                size_t distance_end = line.find_first_of('m', distance_start);
                int distance = std::stoi(line.substr(distance_start, distance_end - distance_start));

                size_t to_stop_start = distance_end + 5;
                size_t to_stop_end = line.find_first_of(',', to_stop_start);
                transport::Stop &to_stop = catalogue_.GetStop(line.substr(to_stop_start, to_stop_end - to_stop_start));

                catalogue_.AddDistance(from_stop, to_stop, distance);

                distance_start = to_stop_end != std::string::npos ? to_stop_end + 1 : std::string::npos;
            }
        }
    }
}