import qbs

Product {
    name: "P7"
    targetName: "p7"

    type: "staticlibrary"
    Depends { name: "cpp" }

    cpp.defines: project.cppDefines.concat([
        "_UNICODE",
        "UNICODE",
        "_FILE_OFFSET_BITS=64",
        "_LARGEFILE64_SOURCE",
    ])
    cpp.cxxFlags: project.cxxFlags.concat([
        "-Wno-variadic-macros",
        "-Wno-long-long",
        "-Wno-missing-field-initializers",
        "-Wno-unknown-pragmas",
        "-Wno-address-of-packed-member",
    ])
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var includePaths: [
        "v5.5/Headers",
        "v5.5/Shared",
        "v5.5/Shared/Platforms/Linux_x86",
    ]
    cpp.includePaths: includePaths;

    files: [
        "v5.5/Headers/*.h",
        "v5.5/Shared/Platforms/Linux_x86/*.h",
        "v5.5/Shared/*.h",
        "v5.5/Sources/*.cpp",
        "v5.5/Sources/*.h",
//        "spdlog/include/spdlog/cfg/*.h",
//        "spdlog/include/spdlog/details/*.h",
//        "spdlog/include/spdlog/fmt/*.h",
//        "spdlog/include/spdlog/fmt/bundled/*.h",
//        "spdlog/include/spdlog/sinks/*.h",
//        "spdlog/include/spdlog/*.h",
//        "spdlog/src/details/*.cpp",
//        "spdlog/src/sinks/*.cpp",
//        "spdlog/src/*.cpp",
    ]

//    excludeFiles: [
//        "spdlog/include/spdlog/details/windows_include.h",
//        "spdlog/include/spdlog/sinks/win_eventlog_sink.h",
//        "spdlog/include/spdlog/sinks/wincolor_sink.h",
//        "spdlog/src/sinks/wincolor_sink.cpp",
//    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: exportingProduct.includePaths
    }
}
