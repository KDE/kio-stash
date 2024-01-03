cmake_minimum_required(VERSION 3.16)

project(KIOFileStash)

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.101.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(
    CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING) #prevents implicit conversion of QString to QUrl
add_definitions(-DTRANSLATION_DOMAIN=\"kio5_stash\")

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
    Test
)

find_package(KF${QT_MAJOR_VERSION} ${KF${QT_MAJOR_VERSION}_MIN_VERSION} REQUIRED COMPONENTS
    Config
    KIO
    DBusAddons
    I18n
)

add_subdirectory(dbus)
add_subdirectory(src)
add_subdirectory(tests)

ki18n_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
