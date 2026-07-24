cmake_minimum_required(VERSION 3.19)
project(mass-audio-converter LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(mass-audio-converter
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_translations(
    TARGETS mass-audio-converter
    TS_FILES
        translations/mass-audio-converter_it_IT.ts
        translations/mass_audio_converter_en_US.ts
)

target_link_libraries(mass-audio-converter
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS mass-audio-converter
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET mass-audio-converter
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
