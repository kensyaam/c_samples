// テストケース記述ファイル
#include "gtest/gtest.h" // googleTestを使用するおまじないはこれだけでOK
// テスト対象関数を呼び出せるようにするのだが
// extern "C"がないとCと解釈されない、意外とハマりがち。
extern "C" {
// #include "target.h"
#include "../../src/target.c"

}

#include "testutil.hpp"

// fixtureNameはテストケース群をまとめるグループ名と考えればよい、任意の文字列
// それ以外のclass～testing::Testまではおまじないと考える
class fixtureName : public ::testing::Test {
protected:
    // fixtureNameでグループ化されたテストケースはそれぞれのテストケース実行前に
    // この関数を呼ぶ。共通の初期化処理を入れておくとテストコードがすっきりする
    virtual void SetUp(){
    }
    // SetUpと同様にテストケース実行後に呼ばれる関数。共通後始末を記述する。
    virtual void TearDown(){
    }
};

// 成功するテストケース。細かい説明はGoogleTestのマニュアルを見てね。
TEST_F(fixtureName, testOk)
{
    EXPECT_EQ(0, function(0));
    EXPECT_EQ(1, function(100));

    printf("testutil_exec_cmd (\"pwd; ls -l\")\n");
    testutil_exec_cmd("pwd; ls -l");

    printf("testutil_exec_cmd_and_read_output (\"whoami; pwd; date\", ...)\n");
    size_t output_size = 0;
    char *output = testutil_exec_cmd_and_read_output("whoami; pwd; date", &output_size);
    printf("output_size: %d, output: \n%s\n", output_size, output);
    free(output);

    printf("testutil_read_file_to_buffer (\"contact.json\", ...)\n");
    size_t file_size = 0;
    char *file = testutil_read_file_to_buffer("contact.json", &file_size);
    printf("file_size: %d, file: %s\n", file_size, file);
    free(file);

    printf("testutil_read_file_to_lines (\"contact.json\", ...)\n");
    size_t line_count = 0;
    char **lines = testutil_read_file_to_lines("contact.json", &line_count);
    for (size_t i = 0; i < line_count; i++) {
        printf("line[%d]: %s", i, lines[i]);
    }
    testutil_free_lines(lines, line_count);
}

// あえて失敗するテストケースも書いておく。
TEST_F(fixtureName, testNg)
{
    EXPECT_EQ(1, function(0));
    EXPECT_EQ(0, function(100));
}

