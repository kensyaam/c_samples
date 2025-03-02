#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gtest/gtest.h"
#include "fff.h"

extern "C" {
// #include "logger.h"
#include "../../src/logger.c"

DEFINE_FFF_GLOBALS;

// malloc のモック定義
FAKE_VALUE_FUNC(void*, malloc_wrapped, size_t);
FAKE_VALUE_FUNC(void*, remalloc_wrapped, void*, size_t);
FAKE_VOID_FUNC(free_wrapped, void*);
}

#define TEST_LOG_FILE "test_logfile.log"


// カスタム malloc の挙動
int malloc_return_null = 0;
void* my_custom_malloc(size_t size) {
    printf("custom_malloc\n"); 
    if (malloc_return_null != 0) {
        malloc_return_null = 0;
        return NULL;
    }

    return malloc(size); // 実際の malloc を使う場合
}

class LoggerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // レポートに出力するプロパティを設定
        testing::Test::RecordProperty("target", "logger.c");
    }

    virtual void SetUp(){
        printf("SetUp\n");
        RESET_FAKE(malloc_wrapped);
        FFF_RESET_HISTORY();
        
        // malloc の動作をカスタム関数に置き換え
        malloc_wrapped_fake.custom_fake = my_custom_malloc;
    }

    virtual void TearDown(){
    }
};

void remove_test_logs() {
    printf("%s\n", __func__);
    for (int i = 0; i < MAX_LOG_FILES; i++) {
        char log_file[256];
        snprintf(log_file, sizeof(log_file), "%s.%d", TEST_LOG_FILE, i);
        remove(log_file);
    }
    remove(TEST_LOG_FILE);
}

TEST_F(LoggerTest, RotateLogs) {
    // レポートに出力するプロパティを設定
    testing::Test::RecordProperty("overview", "rotete_logs()のテスト");
    printf("%s\n", __func__);
    remove_test_logs();

    for (int i = 0; i < MAX_LOG_FILES; i++) {
        FILE *file = fopen(TEST_LOG_FILE, "w");
        fprintf(file, "test%d", i);
        fclose(file);
    
        rotate_logs(TEST_LOG_FILE);
    
        struct stat st;
        char log_file[256];
        for (int j = 0; j <= i; j++) {
            snprintf(log_file, sizeof(log_file), "%s.%d", TEST_LOG_FILE, j);
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
        snprintf(log_file, sizeof(log_file), "%s.%d", TEST_LOG_FILE), i + 1;
        ASSERT_EQ(stat(log_file, &st), -1);
    }

    remove_test_logs();
}

TEST_F(LoggerTest, LogMessageMallocFail) {
    printf("%s\n", __func__);
    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

    printf("Before log_message: %p, %p, %p\n", malloc_wrapped_fake.custom_fake, my_custom_malloc, malloc_wrapped);
    malloc_return_null = 1;
    log_message(LOG_TYPE_INFO, "Test message");
    malloc_return_null = 0;
    printf("After  log_message: %p, %p\n", malloc_wrapped_fake.custom_fake, my_custom_malloc);

    ASSERT_EQ(malloc_wrapped_fake.call_count, 1); // 追加: malloc_wrapped が呼ばれた回数を確認
    ASSERT_EQ(log_queue_size, 0);
}

TEST_F(LoggerTest, LogMessage) {
    printf("%s\n", __func__);
    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

    log_message(LOG_TYPE_INFO, "Test message");

    ASSERT_NE(log_queue_head, nullptr);
    ASSERT_EQ(log_queue_tail, log_queue_head);
    ASSERT_EQ(log_queue_size, 1);
    ASSERT_STREQ(log_queue_head->message, "Test message");

    free(log_queue_head);
    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;
}

TEST_F(LoggerTest, LogThreadFunc) {
    remove_test_logs();

    log_queue_head = NULL;
    log_queue_tail = NULL;
    log_queue_size = 0;

    log_message(LOG_TYPE_INFO, "Test message");

    pthread_t thread;
    log_thread_running = 1;
    pthread_create(&thread, NULL, log_thread_func, NULL);

    sleep(1);

    log_thread_running = 0;
    pthread_cond_signal(&log_cond);
    pthread_join(thread, NULL);

    FILE *file = fopen(LOG_FILE_INFO, "r");
    ASSERT_NE(file, nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    ASSERT_STREQ(buffer, "Test message");

    fclose(file);
    remove_test_logs();
}

TEST_F(LoggerTest, StartStopLogThread) {
    start_log_thread();
    ASSERT_NE(log_thread, (pthread_t)NULL);

    stop_log_thread();
    ASSERT_EQ(log_thread, (pthread_t)NULL);
}

