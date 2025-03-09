#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gtest/gtest.h"
#include "fff.h"

extern "C" {
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

}

#include "testutil.hpp"

// ログファイルの書き込み待ち時間
#define FILE_FLUSH_WAIT_MSEC 300

void remove_test_logs(const char *target_log_file = LOG_FILE_INFO) {
    // printf("%s\n", __func__);
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s.%d", target_log_file, i);
        remove(log_file);
    }
    remove(target_log_file);
}

class LoggerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // レポートに出力するプロパティを設定
        testing::Test::RecordProperty("target", "logger.c");
    }

    virtual void SetUp(){
        RESET_FAKE(malloc_wrapped);
        RESET_FAKE(realloc_wrapped);
        RESET_FAKE(free_wrapped);
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

TEST_F(LoggerTest, RotateLogs) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "rotete_logs()のテスト");

    for (int i = 0; i < MAX_LOG_FILES; i++) {
        FILE *file = fopen(LOG_FILE_INFO, "w");
        fprintf(file, "test%d", i);
        fclose(file);
    
        rotate_logs(LOG_FILE_INFO);
    
        struct stat st;
        char log_file[256];
        for (int j = 0; j <= i; j++) {
            snprintf(log_file, sizeof(log_file), "%s.%d", LOG_FILE_INFO, j);
            printf("j=%d log_file: %s\n", j, log_file);
            ASSERT_EQ(stat(log_file, &st), 0);

            char actual[10];
            char expected[10];
            snprintf(expected, sizeof(expected), "test%d", i - j);
            FILE *file = fopen(log_file, "r");
            fgets(actual, sizeof(actual), file);
            fclose(file);
            printf("i=%d, j=%d, log_file=%s, actual=%s, expected=%s\n", i, j, log_file, actual, expected);
            ASSERT_STREQ(actual, expected);
        }
        snprintf(log_file, sizeof(log_file), "%s.%d", LOG_FILE_INFO, i + 1);
        printf("log_file=%s is expected that is not exist\n", log_file);
        ASSERT_EQ(stat(log_file, &st), -1);
    }

    // remove_test_logs();
}

TEST_F(LoggerTest, LogMessageMallocFail) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "log_message() malloc NGテスト");

    // malloc, free を完全にモック化
    malloc_wrapped_fake.custom_fake = NULL;
    free_wrapped_fake.custom_fake = NULL;

    // mallocが失敗するように設定
    malloc_wrapped_fake.return_val = NULL;

    log_message(LOG_TYPE_INFO, "Test message");

    ASSERT_EQ(malloc_wrapped_fake.call_count, 1);
    ASSERT_EQ(log_queue_size, 0);
}

TEST_F(LoggerTest, LogMessageSuccess) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "log_message() 1メッセージテスト");

    // malloc, free を完全にモック化
    malloc_wrapped_fake.custom_fake = NULL;
    free_wrapped_fake.custom_fake = NULL;

    LogMessage *mock_msg = (LogMessage *)malloc(sizeof(LogMessage));
    malloc_wrapped_fake.return_val = mock_msg;

    log_message(LOG_TYPE_INFO, "Test message");

    ASSERT_EQ(malloc_wrapped_fake.call_count, 1);
    ASSERT_EQ(log_queue_size, 1);
    ASSERT_NE(log_queue_head, nullptr);
    ASSERT_EQ(log_queue_tail, log_queue_head);
    ASSERT_STREQ(log_queue_head->message, "Test message");

    free(mock_msg);
    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;
}

TEST_F(LoggerTest, LogMessageMultipleMessages) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "log_message() 複数メッセージテスト");

    // malloc, free を完全にモック化
    malloc_wrapped_fake.custom_fake = NULL;
    free_wrapped_fake.custom_fake = NULL;

    LogMessage *mock_msg1 = (LogMessage *)malloc(sizeof(LogMessage));
    LogMessage *mock_msg2 = (LogMessage *)malloc(sizeof(LogMessage));
    void* mallocReturnVals[] = {mock_msg1, mock_msg2};
    SET_RETURN_SEQ(malloc_wrapped, mallocReturnVals, 2);

    log_message(LOG_TYPE_INFO, "First message");
    log_message(LOG_TYPE_INFO, "Second message");

    ASSERT_EQ(malloc_wrapped_fake.call_count, 2);
    ASSERT_EQ(log_queue_size, 2);
    ASSERT_NE(log_queue_head, nullptr);
    ASSERT_NE(log_queue_tail, nullptr);
    ASSERT_STREQ(log_queue_head->message, "First message");
    ASSERT_STREQ(log_queue_tail->message, "Second message");

    free(mock_msg1);
    free(mock_msg2);
    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;
}

TEST_F(LoggerTest, LogMessageTypeDebugError) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "log_message() DEBUG/ERRORメッセージテスト");

    // // malloc, free を完全にモック化
    // malloc_wrapped_fake.custom_fake = NULL;
    // free_wrapped_fake.custom_fake = NULL;

    // LogMessage *mock_msg = (LogMessage *)malloc(sizeof(LogMessage));
    // malloc_wrapped_fake.return_val = mock_msg;

    start_log_thread();

    size_t line_count = 0;
    char **lines = NULL;

    log_message(LOG_TYPE_DEBUG, "Test message DEBUG");
    usleep(FILE_FLUSH_WAIT_MSEC);
    lines = testutil_read_file_to_lines(LOG_FILE_DEBUG, &line_count);
    EXPECT_EQ(line_count, 1);
    EXPECT_STREQ(lines[0], "Test message DEBUG");
    testutil_free_lines(lines, line_count);
    
    line_count = 0;
    lines = NULL;
    log_message(LOG_TYPE_ERROR, "Test message ERROR 1\n");
    usleep(FILE_FLUSH_WAIT_MSEC);
    log_message(LOG_TYPE_ERROR, "Test message ERROR 2");
    usleep(FILE_FLUSH_WAIT_MSEC);
    lines = testutil_read_file_to_lines(LOG_FILE_ERROR, &line_count);
    EXPECT_EQ(line_count, 2);
    EXPECT_STREQ(lines[0], "Test message ERROR 1\n");
    EXPECT_STREQ(lines[1], "Test message ERROR 2");
    testutil_free_lines(lines, line_count);

    stop_log_thread();
}

TEST_F(LoggerTest, LogThreadFunc) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", 
        "log_thread_func()のテスト");

    // malloc, free を完全にモック化
    malloc_wrapped_fake.custom_fake = NULL;
    free_wrapped_fake.custom_fake = NULL;

    pthread_t thread;

    // 何もせずに終了すること
    log_thread_running = 0;
    ASSERT_EQ(log_thread_func(NULL), nullptr);

    log_thread_running = 1;
    pthread_create(&thread, NULL, log_thread_func, NULL);
    usleep(FILE_FLUSH_WAIT_MSEC);

    // ログファイルが作成されていないことを確認
    ASSERT_EQ(fopen(LOG_FILE_INFO, "r"), nullptr);

    // mallocの戻り値セット
    LogMessage *mock_msg1 = (LogMessage *)malloc(sizeof(LogMessage));
    LogMessage *mock_msg2 = (LogMessage *)malloc(sizeof(LogMessage));
    LogMessage *mock_msg3 = (LogMessage *)malloc(sizeof(LogMessage));
    void* mallocReturnVals[] = {mock_msg1, mock_msg2, mock_msg3};
    SET_RETURN_SEQ(malloc_wrapped, mallocReturnVals, 3);

    log_message(LOG_TYPE_INFO, "Test message1\n");
    usleep(FILE_FLUSH_WAIT_MSEC);

    FILE *file = fopen(LOG_FILE_INFO, "r");
    EXPECT_NE(file, nullptr);
    EXPECT_EQ(free_wrapped_fake.call_count, 1);
    EXPECT_EQ(free_wrapped_fake.arg0_history[0], mock_msg1);
    fclose(file);

    log_message(LOG_TYPE_INFO, "Test message2\n");
    log_message(LOG_TYPE_INFO, "Test message3\n");
    usleep(FILE_FLUSH_WAIT_MSEC);

    size_t line_count = 0;
    char **lines = NULL;

    lines = testutil_read_file_to_lines(LOG_FILE_INFO, &line_count);
    EXPECT_EQ(line_count, 3);
    EXPECT_STREQ(lines[0], "Test message1\n");
    EXPECT_STREQ(lines[1], "Test message2\n");
    EXPECT_STREQ(lines[2], "Test message3\n");
    testutil_free_lines(lines, line_count);

    EXPECT_EQ(free_wrapped_fake.call_count, 3);
    EXPECT_EQ(free_wrapped_fake.arg0_history[1], mock_msg2);
    EXPECT_EQ(free_wrapped_fake.arg0_history[2], mock_msg3);

    log_thread_running = 0;
    pthread_cond_signal(&log_cond);
    pthread_join(thread, NULL);

    free(mock_msg1);
    free(mock_msg2);
    free(mock_msg3);
}


TEST_F(LoggerTest, StartStopLogThread) {
    start_log_thread();

    stop_log_thread();
    ASSERT_EQ(log_thread_running, 0);

    // ログスレッドから呼ばれた時は何もしないこと
    log_thread_running = 1;
    log_thread = pthread_self();
    stop_log_thread();
    ASSERT_EQ(log_thread_running, 1);
}

