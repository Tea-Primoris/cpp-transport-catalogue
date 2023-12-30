#include <gtest/gtest.h>

#include "../transport-catalogue/transport_catalogue.h"
#include "../transport-catalogue/request_handler.h"
#include "../transport-catalogue/json_reader.h"

class IOTest : public testing::Test {
protected:
    IOTest() : handler_(catalogue_), reader_(catalogue_, handler_) {}

    void SetUp() override {}

    void TearDown() override {}

    transport::Catalogue catalogue_;
    requesthandler::RequestHandler handler_;
    jsonreader::JSONReader reader_;
};

TEST_F(IOTest, TestingTests) {
    std::stringstream input;
    std::getenv("PROJECT_DIR");
}
