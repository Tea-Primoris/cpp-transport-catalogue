#include "transport_catalogue.h"
#include "json_reader.h"

int main() {
    transport::Catalogue catalogue;
    json::Reader reader(catalogue);

    reader.Read(std::cin);
    reader.BuildJSON(std::cout);
}