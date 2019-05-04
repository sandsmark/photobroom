
include(${CMAKE_SOURCE_DIR}/cmake_private/functions.cmake)

find_package(Threads REQUIRED)
find_package(GMock REQUIRED)
find_package(GTest REQUIRED)

include_directories(unit_tests)

qt5_wrap_cpp_for_ut(moced_cpps
                        desktop/models/model_helpers/idx_data_manager.hpp
                        desktop/models/model_helpers/idxdata_deepfetcher.hpp
                        desktop/views/view_impl/view_data_set.hpp
)


addTestTarget(gui
                SOURCES
                    ${moced_cpps}

                    desktop/models/aphoto_info_model.cpp
                    desktop/models/db_data_model.cpp
                    desktop/models/model_helpers/idx_data.cpp
                    desktop/models/model_helpers/idx_data_manager.cpp
                    desktop/models/model_helpers/idxdata_deepfetcher.cpp
                    desktop/models/model_helpers/photos_matcher.cpp

                    desktop/views/view_impl/data.cpp
                    desktop/views/view_impl/model_index_info.cpp
                    desktop/views/view_impl/positions_calculator.cpp
                    desktop/views/view_impl/positions_reseter.cpp
                    desktop/views/view_impl/positions_translator.cpp

                    desktop/utils/model_index_utils.cpp

                    desktop/widgets/tag_editor/helpers/appendable_model_proxy.cpp

                    # model tests:
                    unit_tests/model/appenable_model_proxy_tests.cpp
                    unit_tests/model/idx_data_manager_tests.cpp
                    unit_tests/model/idx_data_tests.cpp

                    # view tests
                    unit_tests/view/data_tests.cpp
                    unit_tests/view/positions_calculator_tests.cpp
                    unit_tests/view/positions_reseter_tests.cpp

                    # helpers:
                    unit_tests/test_helpers/mock_configuration.hpp
                    unit_tests/test_helpers/mock_qabstractitemmodel.hpp

                    unit_tests/test_helpers/internal_task_executor.cpp
                    unit_tests/test_helpers/internal_task_executor.hpp

                    unit_tests/test_helpers/photo_info_model.cpp
                    unit_tests/test_helpers/photo_info_model.hpp

                    # utils:
                    unit_tests/utils/model_index_utils_tests.cpp

                    # main()
                    unit_tests/main.cpp

                LIBRARIES
                     PRIVATE
                        core
                        database
                        photos_crawler
                        Qt5::Core
                        Qt5::Gui
                        Qt5::Widgets
                        ${GMOCK_LIBRARY}
                        ${GTEST_LIBRARY}
                        ${CMAKE_THREAD_LIBS_INIT}

                INCLUDES
                    PRIVATE
                        ${CMAKE_CURRENT_SOURCE_DIR}/desktop/models/model_helpers
                        ${CMAKE_CURRENT_SOURCE_DIR}/desktop/models
                        ${CMAKE_CURRENT_SOURCE_DIR}/desktop
                        ${GMOCK_INCLUDE_DIRS}
                        ${GTEST_INCLUDE_DIRS}
)
