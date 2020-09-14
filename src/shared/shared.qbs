import qbs

Product {
    name: "SharedLib"
    targetName: "shared"

    type: "staticlibrary"

    Depends { name: "cpp" }
    //Depends { name: "Yaml" }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags //.concat(["-fPIC"])
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var exportIncludePaths: [
        "./",
        "./shared",
    ]
    cpp.includePaths: exportIncludePaths;

    files: [
        //"shared/logger/config.cpp",
        //"shared/logger/config.h",
        "shared/logger/format.h",
        "shared/logger/logger.cpp",
        "shared/logger/logger.h",
        "shared/thread/thread_base.cpp",
        "shared/thread/thread_base.h",
        "shared/thread/thread_utils.cpp",
        "shared/thread/thread_utils.h",
        "shared/break_point.h",
        "shared/clife_base.h",
        "shared/clife_ptr.h",
        "shared/list.h",
        "shared/safe_singleton.h",
        "shared/simple_ptr.h",
        "shared/spin_locker.h",
        "shared/steady_timer.h",
        "shared/utils.cpp",
        "shared/utils.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: product.exportIncludePaths
    }
}
