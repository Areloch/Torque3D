set(OPUS_VERSION )
set(OPUS_VERSION_STRING )
set(OPUS_VERSION_MAJOR )
set(OPUS_VERSION_MINOR )
set(OPUS_VERSION_PATCH )



set_and_check(OPUS_INCLUDE_DIR "")
set_and_check(OPUS_INCLUDE_DIRS "")

include(${CMAKE_CURRENT_LIST_DIR}/OpusTargets.cmake)

set(OPUS_LIBRARY Opus::opus)
set(OPUS_LIBRARIES Opus::opus)

check_required_components(Opus)

set(OPUS_FOUND 1)
