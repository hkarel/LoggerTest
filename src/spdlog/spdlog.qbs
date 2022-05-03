import qbs

Product {
    name: "SpdLog"
    targetName: "spdlog"

    type: "staticlibrary"
    Depends { name: "cpp" }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var includePaths: [
        "./",
        "./spdlog/include",
    ]
    cpp.includePaths: includePaths;

    files: [
        "spdlog/include/spdlog/cfg/*.h",
        "spdlog/include/spdlog/details/*.h",
        "spdlog/include/spdlog/fmt/*.h",
        "spdlog/include/spdlog/fmt/bundled/*.h",
        "spdlog/include/spdlog/sinks/*.h",
        "spdlog/include/spdlog/*.h",
        "spdlog/src/details/*.cpp",
        "spdlog/src/sinks/*.cpp",
        "spdlog/src/*.cpp",
    ]

    excludeFiles: [
        "spdlog/include/spdlog/details/windows_include.h",
        "spdlog/include/spdlog/sinks/win_eventlog_sink.h",
        "spdlog/include/spdlog/sinks/wincolor_sink.h",
        "spdlog/src/sinks/wincolor_sink.cpp",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: exportingProduct.includePaths
    }
}
