add_executable(bar main.cpp)

target_link_libraries(bar ${LIBRARY_NAME} pthread)

install(TARGETS bar
# In order to export target, uncomment next line
#   EXPORT ${PROJECT_EXPORT}
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)