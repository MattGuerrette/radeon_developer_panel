qt_add_library(rdp_theme STATIC)

# Must be set before qt_add_qml_module() or the singleton directive is not
# written into the generated qmldir.
set_source_files_properties(Theme.qml Icons.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(rdp_theme
    URI Rdp.Theme
    QML_FILES
        Theme.qml
        Icons.qml
)

devtools_target_options(rdp_theme)

target_link_libraries(rdp_theme
    PUBLIC
        Qt6::Quick
)
