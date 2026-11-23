# The single source of the Silivid product version. It is independent of the
# inherited project(veyra VERSION ...) and of the upstream v2.x tags.
# A release of this version is tagged silivid-v<version>.
set(SILIVID_VERSION "0.1.0-alpha.1")

# Maps a version to the four 16-bit fields of a Windows VERSIONINFO, keeping
# SemVer order: alpha.N = 10000+N, beta.N = 20000+N, rc.N = 30000+N, final = 40000.
function(silivid_version_fields version)
  if(NOT version MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)(-(alpha|beta|rc)\\.(0|[1-9][0-9]*))?$")
    message(FATAL_ERROR "Silivid version must be X.Y.Z or X.Y.Z-(alpha|beta|rc).N: ${version}")
  endif()
  set(core "${CMAKE_MATCH_1};${CMAKE_MATCH_2};${CMAKE_MATCH_3}")
  set(stage "${CMAKE_MATCH_5}")
  set(number "${CMAKE_MATCH_6}")
  foreach(field IN LISTS core)
    string(LENGTH "${field}" digits)
    if(digits GREATER 5 OR field GREATER 65535)
      message(FATAL_ERROR "Silivid version field does not fit a Windows VERSIONINFO: ${version}")
    endif()
  endforeach()

  set(tweak 40000)
  set(flags 0)
  if(stage)
    string(LENGTH "${number}" digits)
    if(digits GREATER 4)
      message(FATAL_ERROR "Silivid pre-release number must be at most 9999: ${version}")
    endif()
    set(base_alpha 10000)
    set(base_beta 20000)
    set(base_rc 30000)
    math(EXPR tweak "${base_${stage}} + ${number}")
    set(flags VS_FF_PRERELEASE)
  endif()
  list(JOIN core "," fields)
  set(SILIVID_VERSION_RC "${fields},${tweak}" PARENT_SCOPE)
  set(SILIVID_VERSION_RC_FLAGS "${flags}" PARENT_SCOPE)
endfunction()

silivid_version_fields("${SILIVID_VERSION}")
