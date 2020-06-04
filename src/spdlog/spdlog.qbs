import qbs
import GccUtl
import QbsUtl

Product {
    name: "SpdLog"
    targetName: "spdlog"

    type: "staticlibrary"
    //destinationDirectory: "./lib"

    Depends { name: "cpp" }

    cpp.archiverName: GccUtl.ar(cpp.toolchainPathPrefix)
    cpp.defines: project.cppDefines
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var exportIncludePaths: [
        "./",
        "./spdlog/include",
    ]
    cpp.includePaths: exportIncludePaths;

    files: [
        "spdlog/include/spdlog/cfg/*.h",
        "spdlog/include/spdlog/details/*.h",
        "spdlog/include/spdlog/fmt/*.h",
        "spdlog/include/spdlog/fmt/bundled/*.h",
        "spdlog/include/spdlog/sinks/*.h",
        "spdlog/include/spdlog/*.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: product.exportIncludePaths
    }
}
