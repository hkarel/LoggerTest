import qbs
import QbsUtl
//import ProbExt
//import qbs.File

Product {
    name: "G3log Test"
    targetName: "g3log-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "G3log" }

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
        "g3log_main.cpp",
        "g3log_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]
}
