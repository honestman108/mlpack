# Define the files we need to compile.
# Anything not in this list will not be compiled into mlpack.
set(SOURCES
  arma_extend.hpp
  SpMat_extra_bones.hpp
  SpMat_extra_meat.hpp
  Mat_extra_bones.hpp
  Mat_extra_meat.hpp
  Cube_extra_bones.hpp
  Cube_extra_meat.hpp
)

# add directory name to sources
set(DIR_SRCS)
foreach(file ${SOURCES})
  set(DIR_SRCS ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endforeach()
# Append sources (with directory name) to list of all mlpack sources (used at
# the parent scope).
set(MLPACK_SRCS ${MLPACK_SRCS} ${DIR_SRCS} PARENT_SCOPE)
