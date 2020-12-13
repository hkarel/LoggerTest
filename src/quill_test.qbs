import qbs
import QbsUtl
//import ProbExt
//import qbs.File

Product {
    name: "Quill Test"
    targetName: "quill-test"
    condition: true

    type: "application"
    consoleApplication: true
    destinationDirectory: "./"

    Depends { name: "cpp" }
    Depends { name: "cppstdlib" }
    Depends { name: "Quill" }
    Depends { name: "SharedLib" }

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
        "quill_main.cpp",
        "quill_test.cpp",
        "hw_monitor.cpp",
        "hw_monitor.h",
        "params_test.h",
    ]
}
