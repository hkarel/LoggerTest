import qbs

Product {
    name: "Quill"
    targetName: "quill"

    type: "staticlibrary"
    Depends { name: "cpp" }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var exportIncludePaths: [
        "./",
        "./quill/quill/include",
    ]
    cpp.includePaths: exportIncludePaths;

    files: [
        "quill/quill/include/quill/bundled/fmt/*.h",
        "quill/quill/include/quill/bundled/invoke/*.h",
        "quill/quill/include/quill/detail/backend/*.h",
        "quill/quill/include/quill/detail/events/*.h",
        "quill/quill/include/quill/detail/misc/*.h",
        "quill/quill/include/quill/detail/serialize/*.h",
        "quill/quill/include/quill/detail/spsc_queue/*.h",
        "quill/quill/include/quill/detail/*.h",
        "quill/quill/include/quill/filters/*.h",
        "quill/quill/include/quill/handlers/*.h",
        "quill/quill/include/quill/*.h",
        "quill/quill/src/bundled/fmt/*.cc",
        "quill/quill/src/bundled/invoke/*.cc",
        "quill/quill/src/detail/backend/*.cpp",
        "quill/quill/src/detail/misc/*.cpp",
        "quill/quill/src/detail/*.cpp",
        "quill/quill/src/handlers/*.cpp",
        "quill/quill/src/*.cpp",
    ]

//    excludeFiles: [
//        "spdlog/include/spdlog/details/windows_include.h",
//        "spdlog/include/spdlog/sinks/win_eventlog_sink.h",
//        "spdlog/include/spdlog/sinks/wincolor_sink.h",
//        "spdlog/src/sinks/wincolor_sink.cpp",
//    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: product.exportIncludePaths
    }
}
