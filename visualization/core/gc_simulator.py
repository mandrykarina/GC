import subprocess
import json
import os
from typing import Dict, Tuple
from datetime import datetime
import logging

logger = logging.getLogger(__name__)

class GCSimulator:
    """Управление запуском C++ сборщиков"""
    
    def __init__(self, rc_executable: str, ms_executable: str, logs_dir: str = './logs'):
        self.rc_executable = rc_executable
        self.ms_executable = ms_executable
        self.logs_dir = logs_dir
        os.makedirs(logs_dir, exist_ok=True)
        self.simulation_history = []
        logger.info(f"GCSimulator initialized")
        logger.info(f" RC executable: {rc_executable}")
        logger.info(f" MS executable: {ms_executable}")

    def run_simulation(self, heap_size: int, num_objects: int,
                      object_size: int, scenario_type: str) -> Tuple[Dict, Dict]:
        """
        ГЛАВНЫЙ МЕТОД - запускает симуляцию
        scenario_type: 'basic', 'linear', 'cyclic', 'cycle_leak', 'cascade', 'cascade_delete'
        """
        
        # Маппинг сценариев на коды (1, 2, 3)
        scenario_map = {
            'basic': 1,
            'cycle_leak': 2,
        }
        
        scenario_num = scenario_map.get(scenario_type, 1)
        heap_size_mb = heap_size // (1024 * 1024)
        
        logger.info("=" * 70)
        logger.info("STARTING SIMULATION")
        logger.info("=" * 70)
        logger.info(f"Scenario: {scenario_type} (code {scenario_num})")
        logger.info(f"Heap Size: {heap_size_mb} MB")
        logger.info(f"Number of Objects: {num_objects}")
        logger.info(f"Object Size: {object_size} bytes")
        logger.info("=" * 70)
        
        try:
            # ЗАПУСКАЕМ RC СБОРЩИК
            rc_result = self._run_gc_simulator(
                self.rc_executable, 'RC', scenario_num, num_objects, object_size, heap_size_mb, scenario_type
            )
            
            # ЗАПУСКАЕМ MS СБОРЩИК
            ms_result = self._run_gc_simulator(
                self.ms_executable, 'MS', scenario_num, num_objects, object_size, heap_size_mb, scenario_type
            )
            
            self._save_json_results(rc_result, ms_result, scenario_type)
            
            # Добавляем в историю
            self.simulation_history.append({
                'timestamp': datetime.now().isoformat(),
                'parameters': {
                    'heap_size': heap_size,
                    'num_objects': num_objects,
                    'object_size': object_size,
                    'scenario_type': scenario_type
                },
                'rc_result': rc_result,
                'ms_result': ms_result
            })
            
            logger.info("SIMULATION COMPLETED SUCCESSFULLY")
            logger.info("=" * 70)
            return rc_result, ms_result
            
        except Exception as e:
            logger.error(f"SIMULATION FAILED: {e}", exc_info=True)
            return self._error_result('RC', str(e)), self._error_result('MS', str(e))

    def _run_gc_simulator(self, executable: str, gc_type: str,
                         scenario_num: int, num_objects: int,
                         object_size: int, heap_size_mb: int,
                         scenario_name: str) -> Dict:
        """
        Запускает один из сборщиков
        Передает аргументы: ./executable scenario num_objects object_size heap_size_mb
        """
        try:
            if not os.path.exists(executable):
                raise FileNotFoundError(f"{gc_type} executable not found: {executable}")
            
            logger.info(f"\nRunning {gc_type} simulator...")
            logger.info(f" Command: {executable} {scenario_num} {num_objects} {object_size} {heap_size_mb}")
            
            # ПЕРЕДАЕМ ПАРАМЕТРЫ В АРГУМЕНТАХ
            result = subprocess.run(
                [executable, str(scenario_num), str(num_objects), str(object_size), str(heap_size_mb)],
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace',
                timeout=30
            )
            
            logger.debug(f"{gc_type} stdout:\n{result.stdout}")
            if result.stderr:
                logger.debug(f"{gc_type} stderr:\n{result.stderr}")
            
            # ПАРСИМ ВЫХОД И ГЕНЕРИРУЕМ ВИЗУАЛИЗАЦИЮ
            return self._parse_gc_output(result.stdout, gc_type, num_objects, object_size, scenario_name)
            
        except FileNotFoundError as e:
            logger.error(f"{gc_type} executable error: {e}")
            return self._error_result(gc_type, str(e))
        except subprocess.TimeoutExpired:
            logger.error(f"{gc_type} simulator timeout")
            return self._error_result(gc_type, "Timeout")
        except Exception as e:
            logger.error(f"{gc_type} simulator error: {e}")
            return self._error_result(gc_type, str(e))

    def _parse_gc_output(self, stdout: str, gc_type: str,
                        num_objects: int, object_size: int,
                        scenario_type: str) -> Dict:
        """
        ПАРСИТ STDOUT от C++ и извлекает статистику
        """
        
        result = {
            'type': gc_type,
            'scenario': scenario_type,
            'success': True,
            'stats': {
                'total_allocated': num_objects * object_size,
                'total_freed': 0,
                'peak_memory': num_objects * object_size,
                'leaked_memory': 0,
                'recovery_percent': 0.0,
                'objects_created': num_objects,
                'objects_left': 0,
                'execution_time_ms': 0.0
            },
            'objects': [],
            'references': []
        }
        
        try:
            # НАЙТИ СТАТИСТИКУ В STDOUT
            start_marker = f'[{gc_type}_STATS]'
            end_marker = f'[/{gc_type}_STATS]'
            
            if start_marker not in stdout:
                logger.warning(f"No stats block found for {gc_type}")
            else:
                start_idx = stdout.find(start_marker)
                end_idx = stdout.find(end_marker)
                
                if start_idx != -1 and end_idx != -1:
                    stats_block = stdout[start_idx + len(start_marker):end_idx]
                    
                    # Парсим каждую строку
                    for line in stats_block.split('\n'):
                        line = line.strip()
                        if not line or line.startswith('['):
                            continue
                        
                        if ':' in line:
                            key, value = line.split(':', 1)
                            key = key.strip()
                            value = value.strip()
                            
                            try:
                                if key == 'objects_created':
                                    result['stats']['objects_created'] = int(value)
                                elif key == 'objects_left':
                                    result['stats']['objects_left'] = int(value)
                                elif key == 'memory_freed':
                                    result['stats']['total_freed'] = int(value)
                                elif key == 'memory_leaked':
                                    result['stats']['leaked_memory'] = int(value)
                                elif key == 'execution_time_ms':
                                    result['stats']['execution_time_ms'] = float(value)
                                elif key == 'scenario':
                                    result['scenario'] = value
                            except ValueError as e:
                                logger.warning(f"Failed to parse {key}={value}: {e}")
            
            logger.info(f"{gc_type} stats parsed:")
            logger.info(f" Objects created: {result['stats']['objects_created']}")
            logger.info(f" Objects left: {result['stats']['objects_left']}")
            logger.info(f" Memory leaked: {result['stats']['leaked_memory']}")
            logger.info(f" Execution time: {result['stats']['execution_time_ms']:.3f} ms")
            
            # Вычисляем процент восстановления
            if result['stats']['objects_created'] > 0:
                freed = result['stats']['objects_created'] - result['stats']['objects_left']
                result['stats']['recovery_percent'] = (freed / result['stats']['objects_created']) * 100
            
            # ГЕНЕРИРУЕМ ОБЪЕКТЫ ДЛЯ ВИЗУАЛИЗАЦИИ (С ЦИКЛАМИ!)
            result['objects'], result['references'] = self._generate_visualization_objects(
                gc_type,
                result['stats']['objects_created'],
                result['stats']['objects_left'],
                scenario_type
            )
            
            result['success'] = True
            logger.info(f"{gc_type} visualization: {len(result['objects'])} objects, {len(result['references'])} references")
            
        except Exception as e:
            logger.error(f"Error parsing {gc_type} output: {e}")
            result['success'] = False
            result.setdefault('errors', []).append(str(e))
        
        return result

    def _generate_visualization_objects(self, gc_type: str,
                                       objects_created: int,
                                       objects_left: int,
                                       scenario_type: str) -> Tuple[list, list]:
        """
        ГЕНЕРИРУЕТ объекты и ссылки для D3.js визуализации
        РАЗНЫЕ ГРАФЫ ДЛЯ РАЗНЫХ СЦЕНАРИЕВ!
        """
        
        objects = []
        references = []
        
        try:
            # Создаем объекты с ПРАВИЛЬНЫМ СТАТУСОМ
            for i in range(objects_created):
                if gc_type == 'RC':
                    # ВСЕ объекты которые не удалились - leaked
                    if i < objects_left:
                        status = 'leaked'
                    else:
                        status = 'deleted'
                else:  # MS
                    # ВСЕ объекты которые не удалились - alive
                    if i < objects_left:
                        status = 'alive'
                    else:
                        status = 'deleted'

                # Корень выделяем красным цветом (объект 0 если он жив)
                is_root = (i == 0 and status != 'deleted')
                
                # Убираем ref_count из отображения для RC
                objects.append({
                    'id': i,
                    'status': status,
                    'size': 64,
                    'is_root': is_root  # Корень выделен
                })
            
            # СОЗДАЕМ РАЗНЫЕ ГРАФЫ В ЗАВИСИМОСТИ ОТ СЦЕНАРИЯ!
            if scenario_type == 'cycle_leak' or scenario_type == 'cyclic':
                # ЦИКЛИЧЕСКИЙ ГРАФ: 0->1->2->...->N-1->0
                logger.info(f"🌀 Generating CYCLIC graph for {scenario_type}")
                
                for i in range(objects_created):
                    from_id = i
                    to_id = (i + 1) % objects_created  # Замыкаем цикл!
                    
                    # Проверяем, осталась ли ссылка (оба объекта должны быть живы)
                    from_alive = from_id < objects_left
                    to_alive = to_id < objects_left
                    
                    if from_alive and to_alive:
                        status = 'active'
                    else:
                        status = 'removed'
                    
                    # ПОСЛЕДНЯЯ ССЫЛКА - ОСОБАЯ (ЗАМЫКАНИЕ ЦИКЛА)
                    is_cycle_closure = (to_id == 0)
                    
                    references.append({
                        'from_id': from_id,
                        'to_id': to_id,
                        'status': status,
                        'is_cycle_closure': is_cycle_closure,  # ✅ НОВОЕ ПОЛЕ!
                        'link_type': 'cycle' 
                    })
                    
                    if is_cycle_closure:
                        logger.debug(f"  🌀 Cycle closure edge: {from_id} -> {to_id} ({status})")
                    else:
                        logger.debug(f"  Edge: {from_id} -> {to_id} ({status})")
                
                logger.info(f"  Created cyclic graph with {objects_created} edges (last edge closes cycle)")
                
            elif scenario_type == 'cascade_delete' or scenario_type == 'cascade':
                # КАСКАДНОЕ ДЕРЕВО: 0->1->2->... (линейная цепь)
                logger.info(f"🌲 Generating CASCADE tree for {scenario_type}")
                
                for i in range(1, objects_created):
                    from_id = i - 1
                    to_id = i
                    
                    from_alive = from_id < objects_left
                    to_alive = to_id < objects_left
                    
                    if from_alive and to_alive:
                        status = 'active'
                    else:
                        status = 'removed'
                    
                    references.append({
                        'from_id': from_id,
                        'to_id': to_id,
                        'status': status,
                        'is_cycle_closure': False,
                        'link_type': 'normal'
                    })
                
                logger.info(f"  Created cascade tree with {objects_created-1} edges")
                
            else:
                # ЛИНЕЙНАЯ ЦЕПЬ (по умолчанию)
                logger.info(f"📏 Generating LINEAR chain for {scenario_type}")
                
                for i in range(1, objects_created):
                    from_id = i - 1
                    to_id = i
                    
                    from_alive = from_id < objects_left
                    to_alive = to_id < objects_left
                    
                    if from_alive and to_alive:
                        status = 'active'
                    else:
                        status = 'removed'
                    
                    references.append({
                        'from_id': from_id,
                        'to_id': to_id,
                        'status': status,
                        'is_cycle_closure': False,
                        'link_type': 'normal'
                    })
                
                logger.info(f"  Created linear chain with {objects_created-1} edges")
            
            logger.debug(f"{gc_type} visualization: {len(objects)} objects, {len(references)} references")
            
        except Exception as e:
            logger.error(f"Error generating visualization: {e}")
        
        return objects, references

    def _error_result(self, gc_type: str, error_msg: str) -> Dict:
        """Возвращает результат ошибки"""
        return {
            'type': gc_type,
            'scenario': 'error',
            'success': False,
            'error': error_msg,
            'stats': {
                'total_allocated': 0,
                'total_freed': 0,
                'peak_memory': 0,
                'leaked_memory': 0,
                'recovery_percent': 0.0,
                'objects_created': 0,
                'objects_left': 0,
                'execution_time_ms': 0.0
            },
            'objects': [],
            'references': []
        }

    def _save_json_results(self, rc_result: Dict, ms_result: Dict, scenario_type: str):
        """СОХРАНЯЕТ JSON РЕЗУЛЬТАТЫ В ФАЙЛЫ"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
            
            # Сохраняем RC результат
            rc_json_path = os.path.join(self.logs_dir, f'rc_{scenario_type}_{timestamp}.json')
            with open(rc_json_path, 'w', encoding='utf-8') as f:
                json.dump(rc_result, f, indent=2, ensure_ascii=False)
            logger.info(f"RC JSON saved: {rc_json_path}")
            
            # Сохраняем MS результат
            ms_json_path = os.path.join(self.logs_dir, f'ms_{scenario_type}_{timestamp}.json')
            with open(ms_json_path, 'w', encoding='utf-8') as f:
                json.dump(ms_result, f, indent=2, ensure_ascii=False)
            logger.info(f"MS JSON saved: {ms_json_path}")
            
        except Exception as e:
            logger.error(f"Failed to save JSON: {e}", exc_info=True)

    def get_history(self) -> list:
        """Возвращает историю симуляций"""
        return self.simulation_history