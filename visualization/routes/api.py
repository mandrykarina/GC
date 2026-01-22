from flask import Blueprint, request, jsonify, current_app
import logging
import os
from core.gc_simulator import GCSimulator

logger = logging.getLogger(__name__)
api_bp = Blueprint('api', __name__)

# ИНИЦИАЛИЗАЦИЯ СИМУЛЯТОРА (вызывается из app.py)
def init_simulator(app):
    """
    Инициализирует GCSimulator с путем к logs папке
    Вызывается в app.py при создании приложения
    """
    try:
        # Определяем пути
        base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        build_dir = os.path.join(base_dir, 'build', 'bin')
        logs_dir = os.path.join(base_dir, 'logs')
        
        # Пути к исполняемым файлам
        rc_exe = os.path.join(build_dir, 'gc_unified.exe')
        ms_exe = os.path.join(build_dir, 'gc_unified.exe')
        
        # ИНИЦИАЛИЗИРУЕМ СИМУЛЯТОР С logs_dir
        app.gc_simulator = GCSimulator(
            rc_executable=rc_exe,
            ms_executable=ms_exe,
            logs_dir=logs_dir  # ПЕРЕДАЕМ logs_dir СЮДА
        )
        logger.info(f"Simulator initialized with logs_dir: {logs_dir}")
        
    except Exception as e:
        logger.error(f"Failed to initialize simulator: {e}", exc_info=True)
        app.gc_simulator = None


@api_bp.route('/config', methods=['GET'])
def get_config():
    """Get configuration"""
    try:
        from config import DevelopmentConfig
        config = DevelopmentConfig()
        return jsonify({
            'scenarios': config.SCENARIOS,
            'defaults': {
                'heap_size': config.DEFAULT_HEAP_SIZE,
                'num_objects': config.DEFAULT_NUM_OBJECTS,
                'object_size': config.DEFAULT_OBJECT_SIZE
            },
            'limits': {
                'max_heap_size': config.MAX_HEAP_SIZE,
                'max_num_objects': config.MAX_NUM_OBJECTS,
                'max_object_size': config.MAX_OBJECT_SIZE,
                'min_heap_size': config.MIN_HEAP_SIZE,
                'min_num_objects': config.MIN_NUM_OBJECTS,
                'min_object_size': config.MIN_OBJECT_SIZE
            }
        })
    except Exception as e:
        logger.error(f"Config error: {e}")
        return jsonify({'error': str(e)}), 500


@api_bp.route('/simulate', methods=['POST'])
def run_simulation():
    """
    Run GC simulation
    """
    try:
        data = request.get_json()
        
        # Extract parameters
        heap_size = data.get('heap_size', 10 * 1024 * 1024)
        num_objects = data.get('num_objects', 10)
        object_size = data.get('object_size', 64)
        scenario_type = data.get('scenario_type', 'basic')
        
        # Validate
        from config import DevelopmentConfig
        config = DevelopmentConfig()
        if scenario_type not in config.SCENARIOS:
            return jsonify({
                'success': False,
                'error': f'Invalid scenario. Use: {list(config.SCENARIOS.keys())}'
            }), 400
        
        logger.info(f"Simulation: heap={heap_size}, objects={num_objects}, size={object_size}, scenario={scenario_type}")
        
        # Get simulator
        if not hasattr(current_app, 'gc_simulator') or current_app.gc_simulator is None:
            return jsonify({'success': False, 'error': 'Simulator not initialized'}), 500
        
        gc_simulator = current_app.gc_simulator
        
        # RUN SIMULATION (gc_simulator сам сохранит JSON в logs_dir)
        rc_result, ms_result = gc_simulator.run_simulation(
            heap_size=heap_size,
            num_objects=num_objects,
            object_size=object_size,
            scenario_type=scenario_type
        )
        
        return jsonify({
            'success': True,
            'parameters': {
                'heap_size': heap_size,
                'num_objects': num_objects,
                'object_size': object_size,
                'scenario_type': scenario_type
            },
            'rc': rc_result,
            'ms': ms_result
        })
        
    except Exception as e:
        logger.error(f"Simulation error: {e}", exc_info=True)
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500


@api_bp.route('/history', methods=['GET'])
def get_history():
    """Get simulation history"""
    try:
        if not hasattr(current_app, 'gc_simulator') or current_app.gc_simulator is None:
            return jsonify({'success': False, 'history': [], 'total': 0})
        
        gc_simulator = current_app.gc_simulator
        history = gc_simulator.simulation_history
        
        return jsonify({
            'success': True,
            'history': history,
            'total': len(history)
        })
    except Exception as e:
        logger.error(f"History error: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500


logger.info('API routes loaded')