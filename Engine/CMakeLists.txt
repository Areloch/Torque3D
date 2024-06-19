message("Engine/CMakeLists.txt is being run!")

# Add engine libs first
add_subdirectory(lib)

# Handle engine sources
add_subdirectory(source)
