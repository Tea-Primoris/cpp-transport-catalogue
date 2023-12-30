#include "transport_catalogue.h"
#include "json_reader.h"

int main() {
    transport::Catalogue catalogue;
    requesthandler::RequestHandler handler(catalogue);
    jsonreader::JSONReader reader(catalogue, handler);

    reader.ReadInput(std::cin);
    Print(handler.GetDocument(), std::cout);
}
