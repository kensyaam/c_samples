import json
import csv
import sys

def extract_date(timestamp):
    """ タイムスタンプから日付（YYYY/MM/DD）を抽出 """
    date_part = timestamp.split("T")[0] if "T" in timestamp else timestamp.split()[0]
    return date_part.replace("-", "/")  # YYYY-MM-DD → YYYY/MM/DD に変換

def parse_gtest_json(json_file, output_file, delimiter=","):
    # output_fileを一旦削除
    with open(output_file, "w", encoding="utf-8"):
        pass

    with open(json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    # JSONに日付情報が含まれているか確認
    test_date = "Unknown"
    if "timestamp" in data:
        test_date = extract_date(data["timestamp"])

    # CSVのヘッダ
    headers = ["Test Suite", "Test Case", "Status", "Date", "Message"]

    summary_headers = ["Test Suite", "Total Tests", "Failures", "Disabled"]

    # 各テストスイートの情報を取得
    for testsuite in data.get("testsuites", []):
        rows = []
        summary_rows = []

        suite_name = testsuite.get("name", "Unknown Suite")
        total_tests = testsuite.get("tests", 0)
        total_failures = testsuite.get("failures", 0)
        total_disabled = testsuite.get("disabled", 0)

        # サマリー情報を追加
        summary_rows.append([suite_name, total_tests, total_failures, total_disabled])

        # 各テストケースを処理
        for testcase in testsuite.get("testsuite", []):
            case_name = testcase.get("name", "Unknown Case")
            status = "PASSED"
            message = ""

            # 失敗時の処理
            if "failures" in testcase and testcase["failures"]:
                status = "FAILED"
                message = testcase["failures"][0].get("message", "").strip()

            rows.append([suite_name, case_name, status, test_date, message])

        # CSV/TSVに保存
        with open(output_file, "a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f, delimiter=delimiter)

            # サマリー情報を書き込む
            writer.writerow(["Test Suite Summary"])
            writer.writerow(summary_headers)
            writer.writerows(summary_rows)
            writer.writerow([])  # 空行

            # テストケース詳細情報を書き込む
            writer.writerow(["Test Case Details"])
            writer.writerow(headers)
            writer.writerows(rows)
            writer.writerow([])  # 空行

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python gtest_json_to_csv.py <input_json> <output_csv> [tsv]")
        sys.exit(1)

    input_json = sys.argv[1]
    output_csv = sys.argv[2]
    delimiter = "\t" if len(sys.argv) > 3 and sys.argv[3].lower() == "tsv" else ","

    parse_gtest_json(input_json, output_csv, delimiter)
    print(f"Converted {input_json} to {output_csv} successfully.")
