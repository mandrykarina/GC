cmake_minimum_required(VERSION 3.10)
project(GC_Unified)

# Установить стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Директория для исполняемых файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Включаемые директории
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/reference_counting/include
    ${CMAKE_SOURCE_DIR}/mark_sweep/include
)

# ============================================
# ОБЩИЕ ФАЙЛЫ (ВЫНЕСЕННЫЕ ИЗ MAIN)
# ============================================
set(COMMON_SOURCES
    common_utils.cpp
    wrapper.cpp
)

# ============================================
# REFERENCE COUNTING - ФАЙЛЫ
# ============================================
set(RC_SOURCES
    reference_counting/src/main.cpp
    reference_counting/src/rc_heap.cpp
    reference_counting/src/reference_counter.cpp
    reference_counting/src/event_logger.cpp
    reference_counting/src/scenario_loader.cpp
    reference_counting/src/rc_logger.cpp
)

# ============================================
# MARK & SWEEP - ФАЙЛЫ
# ============================================
set(MS_SOURCES
    mark_sweep/src/main.cpp
    mark_sweep/src/cascade_deletion_gc.cpp
    mark_sweep/src/mark_sweep_gc.cpp
    mark_sweep/src/performance_test.cpp
)

# ============================================
# ГЛАВНЫЙ ИСПОЛНЯЕМЫЙ ФАЙЛ
# ============================================
add_executable(gc_unified
    main.cpp
    ${COMMON_SOURCES}
    ${RC_SOURCES}
    ${MS_SOURCES}
)

# Опции оптимизации
if(MSVC)
    target_compile_options(gc_unified PRIVATE /W4 /O2)
else()
    target_compile_options(gc_unified PRIVATE -Wall -Wextra -O2 -Wno-unused-parameter)
endif()

# ============================================
# ИНФОРМАЦИЯ О СБОРКЕ
# ============================================
message(STATUS "")
message(STATUS "╔══════════════════════════════════════════════════════════════════════╗")
message(STATUS "║                  GC UNIFIED - CMAKE CONFIGURATION                   ║")
message(STATUS "╠══════════════════════════════════════════════════════════════════════╣")
message(STATUS "║ Project: ${PROJECT_NAME}")
message(STATUS "║ C++ Standard: C++17")
message(STATUS "║ Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "║ Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "║ Build Dir: ${CMAKE_BINARY_DIR}")
message(STATUS "║ Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gc_unified")
message(STATUS "║")
message(STATUS "║ 📦 COMMON UTILITIES (ОБЩИЕ ФАЙЛЫ):") 
message(STATUS "║    - common_utils.h/.cpp (общие структуры и функции)")
message(STATUS "║      • MemoryStats, MemoryConfig, Operation, ParsedScenario")
message(STATUS "║      • Функции конфигурации памяти")
message(STATUS "║      • Функции работы с файлами и JSON")
message(STATUS "║")
message(STATUS "║ 📦 Reference Counting (RC):")
message(STATUS "║    - reference_counting/src/main.cpp")
message(STATUS "║    - reference_counting/src/rc_heap.cpp")
message(STATUS "║    - reference_counting/src/rc_heap.h")
message(STATUS "║    - reference_counting/src/rc_logger.cpp ✨ НОВЫЙ")
message(STATUS "║    - reference_counting/src/rc_logger.h ✨ НОВЫЙ")
message(STATUS "║    - reference_counting/src/reference_counter.cpp")
message(STATUS "║    - reference_counting/src/event_logger.cpp")
message(STATUS "║    - reference_counting/src/scenario_loader.cpp")
message(STATUS "║    → Логирует в файл в формате Mark-Sweep!")
message(STATUS "║")
message(STATUS "║ 📦 Mark & Sweep (MS):")
message(STATUS "║    - mark_sweep/src/main.cpp")
message(STATUS "║    - mark_sweep/src/cascade_deletion_gc.cpp")
message(STATUS "║    - mark_sweep/src/mark_sweep_gc.cpp")
message(STATUS "║    - mark_sweep/src/performance_test.cpp")
message(STATUS "║")
message(STATUS "║ ✨ ЧТО НОВОГО В ЭТОЙ ВЕРСИИ:")
message(STATUS "║    ✅ RCLogger - полный логгер для Reference Counting")
message(STATUS "║    ✅ Логирует в файл [Step N] формате, как Mark-Sweep")
message(STATUS "║    ✅ Отслеживает:")
message(STATUS "║       • ALLOCATE операции с размерами объектов")
message(STATUS "║       • MAKE_ROOT / REMOVE_ROOT")
message(STATUS "║       • ADD_REF / REMOVE_REF")
message(STATUS "║       • Cascade deletion")
message(STATUS "║       • Утечки памяти (LEAKs)")
message(STATUS "║    ✅ Сессионные маркеры для начала/конца работы")
message(STATUS "║")
message(STATUS "║ 🔧 КОМПИЛЯЦИЯ:")
message(STATUS "║    - Опции: -Wall -Wextra -O2 -Wno-unused-parameter")
message(STATUS "║    - Отключены предупреждения о неиспользуемых параметрах")
message(STATUS "║    - Включена оптимизация O2")
message(STATUS "║")
message(STATUS "║ 📊 ВСЕГО ИСХОДНЫХ ФАЙЛОВ:")
message(STATUS "║    - Common: 2 файла")
message(STATUS "║    - Reference Counting: 7 файлов (+ rc_logger.h/.cpp)")
message(STATUS "║    - Mark & Sweep: 4 файла")
message(STATUS "║    - Главный main.cpp: 1 файл")
message(STATUS "║    ─────────────────────────────")
message(STATUS "║    ИТОГО: 14 файлов")
message(STATUS "╚══════════════════════════════════════════════════════════════════════╝")
message(STATUS "")