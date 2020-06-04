import qbs
import QbsUtl
//import ProbExt
//import qbs.File

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
    ]

//    property var test: {
//        console.info("=== project.cudaVersion ===");
//        console.info(agentProbe.cudaVersion);
//    }

}
