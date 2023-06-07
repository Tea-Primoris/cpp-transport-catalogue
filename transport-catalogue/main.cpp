#include "input_reader.h"
#include "transport_catalogue.h"

int main() {
    transport::Catalogue catalogue;
    input_reader::InputHandler update_handler(catalogue);
    update_handler.read_input_commands();
    update_handler.read_output_commands();
}