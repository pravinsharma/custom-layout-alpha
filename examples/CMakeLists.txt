add_executable(flex-example-01-basic-row app/01-basic-row/main.cpp)
add_executable(flex-example-02-basic-column app/02-basic-column/main.cpp)
add_executable(flex-example-03-flex-wrap app/03-flex-wrap/main.cpp)
add_executable(flex-example-04-justify-content app/04-justify-content/main.cpp)
add_executable(flex-example-05-align-items app/05-align-items/main.cpp)
add_executable(flex-example-06-align-content app/06-align-content/main.cpp)
add_executable(flex-example-07-flex-grow-shrink app/07-flex-grow-shrink/main.cpp)
add_executable(flex-example-08-gap app/08-gap/main.cpp)
add_executable(flex-example-09-min-max-constraints app/09-min-max-constraints/main.cpp)
add_executable(flex-example-10-complete-dashboard app/10-complete-dashboard/main.cpp)

foreach(example
    flex-example-01-basic-row
    flex-example-02-basic-column
    flex-example-03-flex-wrap
    flex-example-04-justify-content
    flex-example-05-align-items
    flex-example-06-align-content
    flex-example-07-flex-grow-shrink
    flex-example-08-gap
    flex-example-09-min-max-constraints
    flex-example-10-complete-dashboard
)
    target_link_libraries(${example} PRIVATE flex_layout_lib)
    add_dependencies(${example} Shaders)
endforeach()
