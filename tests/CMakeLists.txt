enable_testing()

file(GLOB files "*.cpp")
foreach(f ${files})
    get_filename_component(n ${f} NAME_WE)
    add_executable(${n} ${n}.cpp)
    target_link_libraries(${n} Linx ${Boost_LIBRARIES})
    add_test(${n} ${n})
endforeach()
