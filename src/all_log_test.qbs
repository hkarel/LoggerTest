import qbs
import QbsUtl

Product {
    name: "All Log Test"
    targetName: "all-log-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "SharedLib" }
    Depends { name: "SpdLog" }
    //Depends { name: "Qt"; submodules: ["core"] }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

//    cpp.includePaths: [
//        "./",
//        "../",
//    ]

    cpp.dynamicLibraries: [
        "pthread",
    ]

    files: [
        "alog_test.cpp",
        "spdlog_test.cpp",
        "all_log_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]

//    property var test: {
//        console.info("=== project.cudaVersion ===");
//        console.info(agentProbe.cudaVersion);
//    }

}
