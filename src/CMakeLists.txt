
set(SOURCE_FILES
        helpPrinter.cpp
        parseUtils.cpp
        parser.cpp
    )


add_library(${PROJECT_NAME} ${SOURCE_FILES})

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

