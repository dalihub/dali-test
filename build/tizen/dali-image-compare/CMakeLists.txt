SET(DALI_IMAGE_COMPARE_SRC_DIR ${ROOT_SRC_DIR}/dali-image-compare)
SET(DALI_TEST_COMMON_DIR ${ROOT_SRC_DIR}/common)
SET(DALI_CONFORMANCE_TESTS_COMMON_DIR ${CMAKE_SOURCE_DIR}/../../dali-conformance-tests/common)

# Source files
SET(SRCS ${DALI_IMAGE_COMPARE_SRC_DIR}/image-compare.cpp)

# Add executable
ADD_EXECUTABLE(dali-image-compare ${SRCS})

# Include directories
TARGET_INCLUDE_DIRECTORIES(dali-image-compare PRIVATE
  ${DALI_TEST_COMMON_DIR}
  ${DALI_CONFORMANCE_TESTS_COMMON_DIR}
)

# Link libraries
TARGET_LINK_LIBRARIES(dali-image-compare ${REQUIRED_PKGS_LDFLAGS})

# Install
INSTALL(TARGETS dali-image-compare DESTINATION ${BINDIR})
