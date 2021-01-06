import qbs
import QbsUtl

Product {
    name: "ALog Test"
    targetName: "alog-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "SharedLib" }
    //Depends { name: "Yaml" }
    //Depends { name: "Qt"; submodules: ["core"] }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

//    cpp.includePaths: [
//        "./",
//        "../",
//    ]

    cpp.rpaths: QbsUtl.concatPaths(
        cppstdlib.path,
        "$ORIGIN/../lib"
    )

    cpp.dynamicLibraries: [
        "pthread",
    ]

    files: [
        "alog/distortneo_format.h",
        "alog_main.cpp",
        "alog_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]

//    property var test: {
//        console.info("=== project.cudaVersion ===");
//        console.info(agentProbe.cudaVersion);
//    }

}
