find_program (VALGRIND_PATH valgrind)

if (VALGRIND_PATH)

   message(STATUS "Found valgrind: ${VALGRIND_PATH}")

	add_custom_target (aqualinkd-memcheck-report)
	add_custom_target (aqualinkd-helgrind-report)

	add_custom_command (TARGET aqualinkd-memcheck-report
                        POST_BUILD
                        COMMAND valgrind --tool=memcheck --leak-check=full --show-reachable=yes 
                                --undef-value-errors=yes --track-origins=no --child-silent-after-fork=no 
                                --trace-children=no --log-file=${CMAKE_CURRENT_BINARY_DIR}/valgrind.aqualinkd.memcheck.log
                                ${CMAKE_CURRENT_BINARY_DIR}/aqualinkd -s /dev/null
						)

	add_custom_command (TARGET aqualinkd-helgrind-report
                        POST_BUILD
                        COMMAND valgrind --tool=helgrind --child-silent-after-fork=no --trace-children=no
                                --log-file=${CMAKE_CURRENT_BINARY_DIR}/valgrind.aqualinkd.helgrind.log
                                ${CMAKE_CURRENT_BINARY_DIR}/aqualinkd -s /dev/null
						)

endif ()