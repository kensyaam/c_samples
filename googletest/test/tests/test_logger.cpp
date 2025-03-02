#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gtest/gtest.h"
#include "fff.h"

extern "C" {
// #include "logger.h"
#include "../../src/logger.c"

DEFINE_FFF_GLOBALS;

// Mock definitions
FAKE_VALUE_FUNC(void*, malloc_wrapped, size_t);
FAKE_VOID_FUNC(free_wrapped, void*);

}

class LoggerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // レポートに出力するプロパティを設定
        testing::Test::RecordProperty("target", "logger.c");
    }

    virtual void SetUp(){
        RESET_FAKE(malloc_wrapped);
        RESET_FAKE(free_wrapped);
        FFF_RESET_HISTORY();
        
        // malloc_wrapped_fake.custom_fake = malloc;
        // free_wrapped_fake.custom_fake = free;
    }

    virtual void TearDown(){
    }
};

void remove_test_logs(const char *target_log_file = LOG_FILE_INFO) {
    printf("%s\n", __func__);
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s.%d", target_log_file, i);
        remove(log_file);
    }
    remove(target_log_file);
}

TEST_F(LoggerTest, RotateLogs) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", "rotete_logs()のテスト");

    remove_test_logs();

    for (int i = 0; i < MAX_LOG_FILES; i++) {
        FILE *file = fopen(LOG_FILE_INFO, "w");
        fprintf(file, "test%d", i);
        fclose(file);
    
        rotate_logs(LOG_FILE_INFO);
    
        struct stat st;
        char log_file[256];
        for (int j = 0; j <= i; j++) {
            snprintf(log_file, sizeof(log_file), "%s.%d", LOG_FILE_INFO, j);
            ASSERT_EQ(stat(log_file, &st), 0);

            char buff[10];
            char expected[10];
            snprintf(expected, sizeof(expected), "test%d", i - j);
            FILE *file = fopen(log_file, "r");
            fgets(buff, sizeof(buff), file);
            fclose(file);
            printf("%s : i=%d, j=%d, buff: %s, expected=%s\n", log_file, i, j, buff, expected);
            ASSERT_STREQ(buff, expected);
        }
        snprintf(log_file, sizeof(log_file), "%s.%d", LOG_FILE_INFO), i + 1;
        ASSERT_EQ(stat(log_file, &st), -1);
    }

    remove_test_logs();
}

TEST_F(LoggerTest, LogMessageMallocFail) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", "log_message() malloc NGテスト");

    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

    malloc_wrapped_fake.return_val = NULL;
    log_message(LOG_TYPE_INFO, "Test message");

    ASSERT_EQ(malloc_wrapped_fake.call_count, 1);
    ASSERT_EQ(log_queue_size, 0);
}

TEST_F(LoggerTest, LogMessageSuccess) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", "log_message() 1メッセージテスト");

    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

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
    testing::Test::RecordProperty("overview", "log_message() 複数メッセージテスト");

    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

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

TEST_F(LoggerTest, LogThreadFunc) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", "log_thread_func()のテスト");

    remove_test_logs();

    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

    pthread_t thread;
    log_thread_running = 1;
    pthread_create(&thread, NULL, log_thread_func, NULL);

    sleep(1);
    ASSERT_EQ(fopen(LOG_FILE_INFO, "r"), nullptr);

    LogMessage *mock_msg1 = (LogMessage *)malloc(sizeof(LogMessage));
    LogMessage *mock_msg2 = (LogMessage *)malloc(sizeof(LogMessage));
    LogMessage *mock_msg3 = (LogMessage *)malloc(sizeof(LogMessage));
    void* mallocReturnVals[] = {mock_msg1, mock_msg2, mock_msg3};
    SET_RETURN_SEQ(malloc_wrapped, mallocReturnVals, 3);

    log_message(LOG_TYPE_INFO, "Test message1\n");

    sleep(1);
    FILE *file = fopen(LOG_FILE_INFO, "r");
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(free_wrapped_fake.call_count, 1);
    ASSERT_EQ(free_wrapped_fake.arg0_history[0], mock_msg1);

    log_message(LOG_TYPE_INFO, "Test message2\n");
    log_message(LOG_TYPE_INFO, "Test message3\n");

    sleep(1);
    file = fopen(LOG_FILE_INFO, "r");
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(free_wrapped_fake.call_count, 3);
    ASSERT_EQ(free_wrapped_fake.arg0_history[1], mock_msg2);
    ASSERT_EQ(free_wrapped_fake.arg0_history[2], mock_msg3);

    log_thread_running = 0;
    pthread_cond_signal(&log_cond);
    pthread_join(thread, NULL);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    ASSERT_STREQ(buffer, "Test message1\n");
    fgets(buffer, sizeof(buffer), file);
    ASSERT_STREQ(buffer, "Test message2\n");
    fgets(buffer, sizeof(buffer), file);
    ASSERT_STREQ(buffer, "Test message3\n");

    fclose(file);
    free(mock_msg1);
    free(mock_msg2);
    free(mock_msg3);
    remove_test_logs();
}


TEST_F(LoggerTest, StartStopLogThread) {
    pthread_t before = log_thread;
    start_log_thread();
    ASSERT_NE(log_thread, before);

    stop_log_thread();
    ASSERT_EQ(log_thread_running, 0);
}

