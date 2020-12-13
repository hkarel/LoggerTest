import qbs

Project {
    name: "Tests"
    references: [
        "all_log_test.qbs",
        "alog_test.qbs",
        "g3log_test.qbs",
        "quill_test.qbs",
        "p7_test.qbs",
        "spdlog_test.qbs",
    ]
}
