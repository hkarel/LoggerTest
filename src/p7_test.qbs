import qbs
import QbsUtl
//import ProbExt
//import qbs.File

Product {
    name: "P7 Test"
    targetName: "p7-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "P7" }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    cpp.includePaths: [
        //"./",
        //"../",
        "shared",
    ]

    cpp.dynamicLibraries: [
        "pthread", "rt",
    ]

    files: [
        "p7_main.cpp",
        "p7_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]
}
