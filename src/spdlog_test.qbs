import qbs
import QbsUtl
//import ProbExt
//import qbs.File

Product {
    name: "SpdLog Test"
    targetName: "spdlog-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "SpdLog" }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    cpp.includePaths: [
        //"./",
        //"../",
        "shared",
    ]

    cpp.dynamicLibraries: [
        "pthread",
    ]

    files: [
        "spdlog_main.cpp",
        "spdlog_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]
}
