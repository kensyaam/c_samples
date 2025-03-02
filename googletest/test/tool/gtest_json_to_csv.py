import json
import csv
import sys

def extract_date(timestamp):
    """ タイムスタンプから日付（YYYY/MM/DD）を抽出 """
    date_part = timestamp.split("T")[0] if "T" in timestamp else timestamp.split()[0]
    return date_part.replace("-", "/")  # YYYY-MM-DD → YYYY/MM/DD に変換

def parse_gtest_json(json_file, output_file, delimiter=","):
    write_encoding = "utf-8"

    # delimiterが","の場合、 output_fileの拡張子がcsvでない場合はcsvに変更
    if delimiter == ",":
        write_encoding = "utf-8-sig"
        if not output_file.lower().endswith(".csv"):
            output_file += ".csv"

    # delimiterが"\t"の場合、 output_fileの拡張子がtsvでない場合はtsvに変更
    if delimiter == "\t" and not output_file.lower().endswith(".tsv"):
        output_file += ".tsv"

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
    summary_headers = ["Target", "Test Suite", "Total Tests", "Failures", "Disabled"]
    headers = ["Test Suite", "Test", "Status", "Date", "Overview", "Failure"]

    # 各テストスイートの情報を取得
    for testsuite in data.get("testsuites", []):
        rows = []
        summary_rows = []

        suite_name = testsuite.get("name", "Unknown Suite")
        total_tests = testsuite.get("tests", 0)
        total_failures = testsuite.get("failures", 0)
        total_disabled = testsuite.get("disabled", 0)
        ts_target = testsuite.get("target", "undefined")

        # サマリー情報を追加
        summary_rows.append([ts_target, suite_name, total_tests, total_failures, total_disabled])

        # 各テストを処理
        for test in testsuite.get("testsuite", []):
            test_name = test.get("name", "Unknown")
            status = "PASSED"

            # 失敗時の処理
            message = ""
            if "failures" in test and test["failures"]:
                status = "FAILED"
                message = test["failures"][0].get("failure", "").strip()

                if delimiter == "\t":
                    # 改行をエスケープ
                    # message = message.replace("\n", "\\n")
                    pass

            # 概要（カスタムフィールド）
            test_overview = test.get("overview", "undefined")
            test_overview = test_overview.encode("latin-1").decode("utf-8")

            if delimiter == "\t":
                # 改行をエスケープ
                # overview = message.replace("\n", "\\n")
                pass

            rows.append([suite_name, test_name, status, test_date, test_overview, message])

        # CSV/TSVに保存
        with open(output_file, "a", newline="", encoding=write_encoding) as f:
            writer = csv.writer(f, delimiter=delimiter)

            # サマリー情報を書き込む
            writer.writerow(["Test Suite Summary"])
            writer.writerow(summary_headers)
            writer.writerows(summary_rows)
            writer.writerow([])  # 空行

            # テスト詳細情報を書き込む
            writer.writerow(["Test Details"])
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
