import qbs

Project {
    name: "Tests"
    references: [
        "all_log_test.qbs",
        "alog_test.qbs",
        "spdlog_test.qbs",
    ]
}
