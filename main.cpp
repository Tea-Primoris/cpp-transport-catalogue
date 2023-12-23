#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    transport::Catalogue catalogue;

    readers::InputReader input_reader(catalogue);
    readers::StatReader stat_reader(catalogue);

    input_reader.ReadInput();
    stat_reader.ReadInput();
}