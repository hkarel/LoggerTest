import qbs
import qbs.File
import qbs.TextFile

Product {
    name: "G3log"
    targetName: "g3log"

    type: "staticlibrary"
    Depends { name: "cpp" }

    Probe {
        id: productProbe

        readonly property string projectBuildDirectory:  project.buildDirectory

        configure: {
            File.makePath(projectBuildDirectory + "/g3log");

            var generated = new TextFile(projectBuildDirectory + "/g3log/generated_definitions.hpp" , TextFile.WriteOnly);
            generated.write("// Dummy file, required for compatibility with CMAKE build system\n");
            generated.close();
        }
    }

    cpp.defines: project.cppDefines.concat([
        //"G3_LOG_FULL_FILENAME=1",
        "G3LOG_DEBUG=DEBUG",
    ])
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var includePaths: [
        "./g3log/src",
        "./g3log/src/g3log",
        project.buildDirectory,
    ]
    cpp.includePaths: includePaths;

    files: [
        "g3log/src/g3log/*.hpp",
        "g3log/src/*.cpp",
        "g3log/src/*.hpp",
    ]

    excludeFiles: [
        "g3log/src/crashhandler_windows.cpp",
        "g3log/src/stacktrace_windows.cpp",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: exportingProduct.includePaths
    }
}
