import qbs
//import "qbs/imports/QbsUtl/qbsutl.js" as QbsUtl

Project {
    name: "LoggerTest (Project)"

    minimumQbsVersion: "1.15.0"
    qbsSearchPaths: ["qbs"]

//    readonly property var projectVersion: projectProbe.projectVersion
//    readonly property string projectGitRevision: projectProbe.projectGitRevision

//    Probe {
//        id: projectProbe
//        property var projectVersion;
//        property string projectGitRevision;

//        readonly property string projectBuildDirectory:  project.buildDirectory
//        readonly property string projectSourceDirectory: project.sourceDirectory

//        configure: {
//            projectVersion = QbsUtl.getVersions(projectSourceDirectory + "/VERSION");
//            projectGitRevision = QbsUtl.gitRevision(projectSourceDirectory);
//        }
//    }

    property var cppDefines: {
        var def = [
//            "VERSION_PROJECT=\"" + projectVersion[0] + "\"",
//            "VERSION_PROJECT_MAJOR=" + projectVersion[1],
//            "VERSION_PROJECT_MINOR=" + projectVersion[2],
//            "VERSION_PROJECT_PATCH=" + projectVersion[3],
//            "GIT_REVISION=\"" + projectGitRevision + "\"",
        ];

        if (qbs.buildVariant === "release")
            def.push("NDEBUG");

        return def;
    }

    property var cxxFlags: [
        "-ggdb3",
        "-Wall",
        "-Wextra",
        "-Wdangling-else",
        "-Wno-unused-parameter",
        "-Wno-variadic-macros",
    ]
    property string cxxLanguageVersion: "c++14"

    references: [
        "src/shared/shared.qbs",
        "src/spdlog/spdlog.qbs",
        "src/g3log/g3log.qbs",
        //"src/yaml/yaml.qbs",
        "src/tests.qbs",
    ]
}
