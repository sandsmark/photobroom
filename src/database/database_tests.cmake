
include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)

find_package(Qt6 REQUIRED COMPONENTS Sql)

addTestTarget(database
                SOURCES
                    backends/sql_backends/generic_sql_query_constructor.cpp
                    backends/sql_backends/sql_filter_query_generator.cpp
                    backends/sql_backends/query_structs.cpp
                    database_tools/implementation/json_to_backend.cpp
                    database_tools/implementation/series_detector.cpp
                    implementation/aphoto_change_log_operator.cpp
                    implementation/filter.cpp
                    implementation/photo_data.cpp
                    implementation/photo_info.cpp
                    implementation/photo_types.cpp
                    # memory backend linked

                    # sql tests:
                    unit_tests/generic_sql_query_constructor_tests.cpp
                    unit_tests/sql_filter_query_generator_tests.cpp

                    # tools tests:
                    unit_tests/json_to_backend_tests.cpp
                    unit_tests/series_detector_tests.cpp
                    unit_tests/tag_info_collector_tests.cpp

                    # backends:
                    unit_tests/memory_backend_tests.cpp

                    # main()
                    unit_tests/main.cpp

                LIBRARIES
                    core
                    database
                    database_memory_backend
                    sample_dbs
                    Qt::Core
                    Qt::Gui
                    Qt::Sql
                    GTest::gtest
                    GTest::gmock

                INCLUDES
                    backends/sql_backends
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/backends/sql_backends
)
