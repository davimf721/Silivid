# macOS arm64 build (Silivid port, Phase 1). Included from the root
# CMakeLists.txt before any Windows target is declared.
# Inventory and blockers: docs/macos/PHASE1.md.

if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  message(FATAL_ERROR "The macOS build targets Apple Silicon only (got ${CMAKE_SYSTEM_PROCESSOR})")
endif()

# Windows-only parts. They have no macOS implementation, so asking for one is a
# configure error instead of a silently ignored switch.
set(VEYRA_MACOS_UNAVAILABLE_OPTIONS
  VEYRA_ENABLE_D3D12             "Direct3D 12 / DXGI renderer and HLSL shaders"
  VEYRA_ENABLE_D3D12_DEBUG       "Direct3D 12 debug layer"
  VEYRA_ENABLE_NGX               "NVIDIA NGX host"
  VEYRA_ENABLE_EXPERIMENTAL_DLSSNR "NVIDIA DLSS NR adapter"
  VEYRA_ENABLE_DLSS_SR           "NVIDIA DLSS Super Resolution"
  VEYRA_ENABLE_DLSS_FG           "NVIDIA DLSS Frame Generation"
  VEYRA_ENABLE_NVOF              "NVIDIA Optical Flow"
  VEYRA_ENABLE_DIRECTSHOW        "DirectShow virtual camera"
  VEYRA_ENABLE_MEDIA_FOUNDATION  "Media Foundation capture and decode"
  VEYRA_ENABLE_WASAPI            "WASAPI audio"
  VEYRA_ENABLE_WINDOWS_INSTALLER "Windows installer and portable packaging"
  VEYRA_ENABLE_REMOTEPLAY        "PS5 Remote Play backend (Chiaki, Win32 links)"
  VEYRA_ENABLE_MOONLIGHT         "Moonlight client (Winsock transport)"
  VEYRA_ENABLE_XBOX              "Xbox streaming backend"
)
while(VEYRA_MACOS_UNAVAILABLE_OPTIONS)
  list(POP_FRONT VEYRA_MACOS_UNAVAILABLE_OPTIONS veyra_option veyra_option_label)
  option(${veyra_option} "${veyra_option_label} (unavailable on macOS)" OFF)
  if(${veyra_option})
    message(FATAL_ERROR "${veyra_option}=ON: ${veyra_option_label} is not available on macOS. See docs/macos/PHASE1.md.")
  endif()
endwhile()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(PkgConfig REQUIRED)
pkg_check_modules(VEYRA_FFMPEG REQUIRED IMPORTED_TARGET
  libavcodec libavformat libavutil libswscale libswresample)

# Sources that build unchanged and whose symbols all resolve inside this set.
add_library(veyra_portable STATIC
  src/base/NgxResult.cpp
  src/beauty/OnnxLite.cpp
  src/beauty/TfliteLite.cpp
  src/engine/EffectChain.cpp
  src/engine/ReplayBuffer.cpp
  src/parity/RenoDxParityCodec.cpp
  src/pipeline/ColorGradeTables.cpp
  src/pipeline/ResetCoordinator.cpp
  src/pipeline/SmaaLookupTables.cpp
  src/remoteplay/AnnexB.cpp
  src/remoteplay/SessionInbox.cpp
  src/remoteplay/Timeline.cpp
  src/remoteplay/Validation.cpp
  src/remoteplay/VideoIngress.cpp
  src/ui/QmlDataDirectory.cpp
)
target_include_directories(veyra_portable PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(veyra_portable PUBLIC Threads::Threads)

add_library(veyra_portable_media STATIC
  src/engine/PosterFrame.cpp
  src/sink/CaptureAudioDsp.cpp
)
target_link_libraries(veyra_portable_media PUBLIC veyra_portable PkgConfig::VEYRA_FFMPEG)

add_library(veyra_portable_qt STATIC
  src/ui/PlaylistModel.cpp include/veyra/ui/PlaylistModel.h
  src/ui/QmlDiskCache.cpp
)
set_target_properties(veyra_portable_qt PROPERTIES AUTOMOC ON)
target_link_libraries(veyra_portable_qt PUBLIC veyra_portable Qt6::Core)

# Links every portable object plus Qt and FFmpeg, so an unresolved symbol hidden
# inside a static archive fails the build.
add_executable(veyra_macos_build_probe tools/macos_build_probe/main.cpp)
target_link_libraries(veyra_macos_build_probe PRIVATE
  "$<LINK_LIBRARY:WHOLE_ARCHIVE,veyra_portable>"
  "$<LINK_LIBRARY:WHOLE_ARCHIVE,veyra_portable_media>"
  "$<LINK_LIBRARY:WHOLE_ARCHIVE,veyra_portable_qt>"
  PkgConfig::VEYRA_FFMPEG Threads::Threads Qt6::Core Qt6::Qml)

enable_testing()
add_test(NAME veyra_macos_build_probe COMMAND veyra_macos_build_probe)

set(VEYRA_MACOS_TESTS
  veyra_adaptive_fg_multiplier_tests tests/unit/AdaptiveFgMultiplierTests.cpp
  veyra_audio_gain_tests             tests/unit/AudioGainTests.cpp
  veyra_buffering_gate_tests         tests/unit/BufferingGateTests.cpp
  veyra_capture_capability_tests     tests/unit/CaptureCapabilityTests.cpp
  veyra_color_grade_tests            tests/unit/ColorGradeTableTests.cpp
  veyra_effect_availability_tests    tests/unit/EffectAvailabilityTests.cpp
  veyra_export_timeline_tests        tests/unit/ExportTimelineTests.cpp
  veyra_frame_pacing_repair_tests    tests/unit/FramePacingRepairTests.cpp
  veyra_game_mode_policy_tests       tests/unit/GameModePolicyTests.cpp
  veyra_hdr_display_state_tests      tests/unit/HdrDisplayStateTests.cpp
  veyra_parity_tests                 tests/unit/ParityCpuReference.cpp
  veyra_realtime_preview_tests       tests/unit/RealtimePreviewSchedulingTests.cpp
  veyra_nr_antiflicker_tests         tests/integration/NrAntiFlickerTests.cpp
  veyra_nr_auto_controller_tests     tests/perf/AutoNrControllerTests.cpp
  veyra_remoteplay_core_tests        tests/remoteplay/CoreTests.cpp
  veyra_remoteplay_trigger_filter_tests tests/remoteplay/TriggerFilterTests.cpp
)
while(VEYRA_MACOS_TESTS)
  list(POP_FRONT VEYRA_MACOS_TESTS veyra_test veyra_test_source)
  add_executable(${veyra_test} ${veyra_test_source})
  target_link_libraries(${veyra_test} PRIVATE veyra_portable)
  add_test(NAME ${veyra_test} COMMAND ${veyra_test})
  set_tests_properties(${veyra_test} PROPERTIES TIMEOUT 60)
endwhile()

# These tests refuse to run unless their scratch directory is new.
function(veyra_macos_add_scratch_test name source library)
  set(scratch "${CMAKE_CURRENT_BINARY_DIR}/test-scratch/${name}")
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE ${library})
  add_test(NAME ${name}_clean COMMAND "${CMAKE_COMMAND}" -E rm -rf "${scratch}")
  add_test(NAME ${name} COMMAND ${name} "${scratch}")
  set_tests_properties(${name}_clean PROPERTIES FIXTURES_SETUP ${name}_scratch)
  set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED ${name}_scratch TIMEOUT 60)
endfunction()

veyra_macos_add_scratch_test(veyra_qml_data_tests tests/unit/QmlDataDirectoryTests.cpp veyra_portable)
veyra_macos_add_scratch_test(veyra_playlist_tests tests/unit/PlaylistModelTests.cpp veyra_portable_qt)
veyra_macos_add_scratch_test(veyra_qml_cache_tests tests/unit/QmlDiskCacheTests.cpp veyra_portable_qt)
