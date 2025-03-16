// テストケース記述ファイル
#include "gtest/gtest.h" // googleTestを使用するおまじないはこれだけでOK
// テスト対象関数を呼び出せるようにするのだが
// extern "C"がないとCと解釈されない、意外とハマりがち。
extern "C" {
// #include "target.h"
#include "../../src/target.c"

}

#include "testbase.hpp"

// ---------------------------------------------------

// WithParamInterface<int>
class WithParamSample1 : public TBase, 
        public testing::WithParamInterface<int> {
protected:
    virtual void SetUp(){
        TBase::SetUp();
    }
    virtual void TearDown(){
        TBase::TearDown(); 
    }
};

// 引数
//   テストインスタンスの名前（任意）
//   テストフィクスチャクラスの名前
//   パラメータのリスト（::testing::Values、::testing::Rangeなどを使用）
INSTANTIATE_TEST_SUITE_P(MyTestInstance, WithParamSample1, 
    ::testing::Values(0, 1, 2));

TEST_P(WithParamSample1, testOk)
{
    int input = GetParam();
    printf("input: %d\n", input);
    EXPECT_EQ((input == 0) ? 0 : 1, function(input));
    EXPECT_EQ(0, function(0));
    EXPECT_EQ(1, function(100));
}

// ---------------------------------------------------
struct TestParam2 {
    int input;
    std::string expected;
};

// WithParamInterface<TestParam2>
class WithParamSample2 : public TBase, 
        public testing::WithParamInterface<TestParam2> {
protected:
    virtual void SetUp(){
        TBase::SetUp();
    }
    virtual void TearDown(){
        TBase::TearDown(); 
    }
};

INSTANTIATE_TEST_SUITE_P(MyTestInstance2, WithParamSample2, 
    ::testing::Values(
        TestParam2{0, "0"},
        TestParam2{1, "1"},
        TestParam2{2, "1"}
    ));

TEST_P(WithParamSample2, testOk)
{
    TestParam2 param = GetParam();
    printf("input: %d, expected: %s\n", param.input, param.expected.c_str());
    int input = param.input;
    std::string expected = param.expected;
    EXPECT_EQ(atoi(expected.c_str()), function(input));
}

// ---------------------------------------------------
struct TestParam3 {
    const char *url;
    const char *method;
    const char *post_data;
    int content_length;
    const char *content_type;
    int expected_status_code;
    const char *expected_data;
    // 文字列出力関数
    friend std::ostream& operator<<(std::ostream& os, const TestParam3& param) {
        return os << "url: " << param.url << ", method: " << param.method << ", post_data: " << param.post_data << ", content_length: " << param.content_length << ", content_type: " << param.content_type << ", expected_status_code: " << param.expected_status_code << ", expected_data: " << param.expected_data;
    }
};

// WithParamInterface<TestParam3>
class WithParamSample3 : public TBase, 
        public testing::WithParamInterface<TestParam3> {
protected:
    virtual void SetUp(){
        TBase::SetUp();
    }
    virtual void TearDown(){
        TBase::TearDown(); 
    }
};

INSTANTIATE_TEST_SUITE_P(MyTestInstance3, WithParamSample3, 
    ::testing::Values(
        TestParam3{"http://localhost:8000/aaa", "GET", NULL, -1, NULL, 200, "Hello, World!\n"},
        TestParam3{"http://localhost:8000/bbb", "GET", NULL, -1, NULL, 200, "Hello, World!\n"},
        TestParam3{"http://localhost:8000/hello", "POST", "{\"aaa\":\"foo\"}", -1, "application/json", 200, "{\"greeting\":\"Hello, World!\"}"}
    ));


TEST_P(WithParamSample3, testUtilCurl) {
    TestParam3 param = GetParam();
    const char *url = param.url;
    const char *method = param.method;
    const char *post_data = param.post_data;
    int content_length = param.content_length;
    const char *content_type = param.content_type;
    int expected_status_code = param.expected_status_code;
    const char *expected_data = param.expected_data;
    printf("url: %s, method: %s, post_data: %s, content_length: %d, content_type: %s, \nexpected_status_code: %d, expected_data: %s\n", url, method, post_data, content_length, content_type, expected_status_code, expected_data);

    char headers[2][256];
    int header_count = 0;
    if (content_type != NULL) {
        snprintf(headers[header_count], sizeof(headers[header_count]), "Content-Type: %s", content_type);
        header_count++;
    }
    if (content_length >= 0) {
        snprintf(headers[header_count], sizeof(headers[header_count]), "Content-Length: %d", content_length);
        header_count++;
    }
    const char *header_ptrs[2];
    for (int i = 0; i < header_count; i++) {
        header_ptrs[i] = headers[i];
    }

    TestUtilResponseData* response = testutil_http_request(method, url, header_ptrs, header_count, post_data);

    EXPECT_NE(response, nullptr);
    if (response != NULL) {
        // printf("response status_code: %d, data: %s\n", response->status_code, response->data);
        // printf("expected status_code: %d, data: %s\n", expected_status_code, expected_data);
        EXPECT_EQ(response->status_code, expected_status_code); // 期待するステータスコードを検証
        EXPECT_STREQ(response->data, expected_data);
    }
    testutil_free_response_data(response);
}

