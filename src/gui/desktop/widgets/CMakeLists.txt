
find_package(Qt5 REQUIRED COMPONENTS QuickWidgets Widgets)

set(SRCS
    collection_dir_scan_dialog.cpp
    color_pick_button.cpp
    media_preview.cpp
    multi_value_line_edit.cpp
    photo_properties.cpp
    table_widget_with_limited_drop.cpp
    tasks_view_widget.cpp
    project_creator/project_creator_dialog.cpp
    series_detection/series_detection.cpp
    tag_editor/tag_editor_widget.cpp
    tag_editor/helpers/tags_item_delegate.cpp
    tag_editor/helpers/tags_operator.cpp
    tag_editor/helpers/tags_view.cpp
    tag_editor/helpers/tags_model.cpp
)

set(HEADERS
    collection_dir_scan_dialog.hpp
    color_pick_button.hpp
    media_preview.hpp
    multi_value_line_edit.hpp
    photo_properties.hpp
    table_widget_with_limited_drop.hpp
    tasks_view_widget.hpp
    project_creator/project_creator_dialog.hpp
    series_detection/series_detection.hpp
    tag_editor/tag_editor_widget.hpp
    tag_editor/helpers/itags_operator.hpp
    tag_editor/helpers/tags_item_delegate.hpp
    tag_editor/helpers/tags_operator.hpp
    tag_editor/helpers/tags_view.hpp
    tag_editor/helpers/tags_model.hpp
)

#set of file to be moced
set(TO_MOC
    collection_dir_scan_dialog.hpp
    media_preview.hpp
    photo_properties.hpp
    project_creator/project_creator_dialog.hpp
    series_detection/series_detection.hpp
    tag_editor/tag_editor_widget.hpp
    tag_editor/helpers/tags_model.hpp
)

source_group(files .*)
source_group(filters_editor .*\/filters_editor\/.* )
source_group(project_creator .*\/project_creator\/.* )
source_group(tag_editor .*\/tag_editor\/.* )
source_group(generated_files \(moc.*\)|\(ui_.*\)|\(qrc_.*\) )

qt5_wrap_cpp(MOCED ${TO_MOC})

add_library(gui_widgets OBJECT ${SRCS} ${MOCED} ${HEADERS})
set_target_properties(gui_widgets PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(gui_widgets
                        PRIVATE
                            core
                            database
                            face_recognition
                            photos_crawler
                            project_utils
                            quick_views
                            system
                            KWidgetsAddons
                            Qt::Widgets
                            Qt::QuickWidgets
)

target_include_directories(gui_widgets
                            SYSTEM PRIVATE
                                ${CMAKE_SOURCE_DIR}/src/gui/
                                ${CMAKE_SOURCE_DIR}/src/gui/desktop
)
