SET(DALI_OVERRIDE_SRC_DIR ${ROOT_SRC_DIR}/dali-override)

# Source files
SET(SRCS
${DALI_OVERRIDE_SRC_DIR}/animation-override.cpp
${DALI_OVERRIDE_SRC_DIR}/system-library-override.cpp
)

# Add shared library
# Use C language to avoid C++ exception handling and GLIBC version issues
ADD_LIBRARY(dali-override SHARED ${SRCS})

# Set library output name to libdali-override.so
SET_TARGET_PROPERTIES(dali-override PROPERTIES
  PREFIX "lib"
  OUTPUT_NAME "dali-override"
)

# Include directories
TARGET_INCLUDE_DIRECTORIES(dali-override PRIVATE
  ${DALI_OVERRIDE_SRC_DIR}
)

# Minimal link flags - allow undefined symbols (resolved at runtime via LD_PRELOAD)
# Don't link against dali libs to avoid pulling in GLIBC versioned symbols
TARGET_LINK_LIBRARIES(dali-override dl)

# Allow undefined symbols - they will be resolved at runtime
SET_TARGET_PROPERTIES(dali-override PROPERTIES
  LINK_FLAGS "-Wl,--allow-shlib-undefined"
)

# Install library to lib directory
INSTALL(TARGETS dali-override DESTINATION ${PREFIX}/lib)
