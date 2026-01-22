"""
Configuration for GC Visualizer
✅ Класс конфигурации для Flask
"""

import os
from pathlib import Path

class DevelopmentConfig:
    """Development configuration"""
    
    # Scenarios
    SCENARIOS = {
        'basic': 'Basic Allocation',
        'cycle_leak': 'Circular References',
    
    }
    
    # Default parameters
    DEFAULT_HEAP_SIZE = 33554432  # 32 MB in bytes
    DEFAULT_NUM_OBJECTS = 20
    DEFAULT_OBJECT_SIZE = 64
    
    # Limits
    MIN_HEAP_SIZE = 1024 * 1024  # 1 MB
    MAX_HEAP_SIZE = 1024 * 1024 * 1024  # 1 GB
    MIN_NUM_OBJECTS = 1
    MAX_NUM_OBJECTS = 10000
    MIN_OBJECT_SIZE = 1
    MAX_OBJECT_SIZE = 10000
    
    # C++ executable path
    CPP_EXECUTABLE = os.path.join(
        os.path.dirname(__file__),
        '../build/bin/gc_unified.exe'
    )
    
    # Logs directory
    LOGS_DIR = os.path.join(os.path.dirname(__file__), '../logs')
    
    # Debug mode
    DEBUG = True
    SAVE_JSON = True


# Для обратной совместимости (если где-то используются старые переменные)
SCENARIOS = DevelopmentConfig.SCENARIOS
DEFAULTS = {
    'heap_size': DevelopmentConfig.DEFAULT_HEAP_SIZE,
    'num_objects': DevelopmentConfig.DEFAULT_NUM_OBJECTS,
    'object_size': DevelopmentConfig.DEFAULT_OBJECT_SIZE
}
CPP_EXECUTABLE = DevelopmentConfig.CPP_EXECUTABLE
LOGS_DIR = DevelopmentConfig.LOGS_DIR
DEBUG = DevelopmentConfig.DEBUG
SAVE_JSON = DevelopmentConfig.SAVE_JSON
