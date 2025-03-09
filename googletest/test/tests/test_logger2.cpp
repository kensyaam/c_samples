#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gtest/gtest.h"
#include "fff.h"

extern "C" {
#include <sys/stat.h>
#include "logger_int.h"

// テスト用にログファイル名を変更
#undef LOG_FILE_INFO
#define LOG_FILE_INFO "test/tmp/logfile_info.log"
#undef LOG_FILE_DEBUG
#define LOG_FILE_DEBUG "test/tmp/logfile_debug.log"
#undef LOG_FILE_ERROR
#define LOG_FILE_ERROR "test/tmp/logfile_error.log"

#include "../../src/logger.c"

DEFINE_FFF_GLOBALS;

// Mock definitions
FAKE_VALUE_FUNC(void*, malloc_wrapped, size_t);
FAKE_VALUE_FUNC(void*, realloc_wrapped, void *, size_t);
FAKE_VOID_FUNC(free_wrapped, void*);

FAKE_VALUE_FUNC(FILE*, fopen, const char*, const char*);
FAKE_VALUE_FUNC(int, fclose, FILE*);
FAKE_VALUE_FUNC_VARARG(int, fprintf, FILE*, const char*, ...);
// FAKE_VALUE_FUNC(int, stat, const char*, struct stat*);
// FAKE_VALUE_FUNC(int, rename, const char*, const char*);
FAKE_VALUE_FUNC_VARARG(int, snprintf, char *, size_t, const char *, ...);

}

#include "testutil.hpp"

// ログファイルの書き込み待ち時間
#define FILE_FLUSH_WAIT_MSEC 200

void remove_test_logs(const char *target_log_file = LOG_FILE_INFO) {
    // printf("%s\n", __func__);
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s.%d", target_log_file, i);
        remove(log_file);
    }
    remove(target_log_file);
}

static long custum_fake_stat_st_size = 0;
extern "C" int custom_fake_stat(const char *path, struct stat *st) {
    st->st_size = custum_fake_stat_st_size;
    return 0;
}

class LoggerTest2 : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // レポートに出力するプロパティを設定
        testing::Test::RecordProperty("target", "logger.c");
    }

    virtual void SetUp(){
        RESET_FAKE(malloc_wrapped);
        RESET_FAKE(realloc_wrapped);
        RESET_FAKE(free_wrapped);
        RESET_FAKE(fopen);
        RESET_FAKE(fclose);
        RESET_FAKE(fprintf);
        // RESET_FAKE(stat);
        // RESET_FAKE(rename);
        // RESET_FAKE(snprintf);
        FFF_RESET_HISTORY();

        //  デフォルトのmalloc, realloc, freeを使う
        malloc_wrapped_fake.custom_fake = malloc;
        realloc_wrapped_fake.custom_fake = realloc;
        free_wrapped_fake.custom_fake = free;

        // 内部変数を初期化
        log_mutex = PTHREAD_MUTEX_INITIALIZER;
        log_cond = PTHREAD_COND_INITIALIZER;
        log_queue_head = NULL;
        log_queue_tail = NULL;
        log_queue_size = 0;
        log_thread_running = 1;

        remove_test_logs(LOG_FILE_INFO);
        remove_test_logs(LOG_FILE_DEBUG);
        remove_test_logs(LOG_FILE_ERROR);
    }

    virtual void TearDown(){
    }
};

TEST_F(LoggerTest2, FopenError) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "rotete_logs()のテスト");

    start_log_thread();

    // rotete_logsが呼ばれないように設定
    // fopenが失敗するように設定
    // stat_fake.custom_fake = custom_fake_stat;
    // custum_fake_stat_st_size = MAX_LOG_SIZE - 1;
    fopen_fake.return_val = NULL;

    log_message(LOG_TYPE_INFO, "Test message\n");
    usleep(FILE_FLUSH_WAIT_MSEC);

    // EXPECT_EQ(stat_fake.call_count, 1);
    EXPECT_EQ(fopen_fake.call_count, 1);
    EXPECT_EQ(fopen_fake.arg0_history[0], LOG_FILE_INFO);
    EXPECT_EQ(free_wrapped_fake.call_count, 1);
    EXPECT_EQ(fprintf_fake.call_count, 0);
    EXPECT_EQ(fclose_fake.call_count, 0);
    // EXPECT_EQ(rename_fake.call_count, 0);

    // rotete_logsが呼ばれるように設定
    // fopenが成功するように設定
    // stat_fake.custom_fake = custom_fake_stat;
    // custum_fake_stat_st_size = MAX_LOG_SIZE;
    // fopen_fake.return_val = (FILE*)0x1234;

    // log_message(LOG_TYPE_INFO, "Test message\n");
    // usleep(FILE_FLUSH_WAIT_MSEC);

    // // EXPECT_EQ(stat_fake.call_count, 2);
    // EXPECT_EQ(fopen_fake.call_count, 2);
    // EXPECT_EQ(free_wrapped_fake.call_count, 2);
    // EXPECT_EQ(fprintf_fake.call_count, 1);
    // EXPECT_EQ(fclose_fake.call_count, 1);
    // EXPECT_GT(rename_fake.call_count, 1);

    stop_log_thread();    
}

