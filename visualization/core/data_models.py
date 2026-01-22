"""
Модели данных для фронтенда
"""
from dataclasses import dataclass, asdict
from typing import List, Dict, Optional
from enum import Enum


class ObjectStatus(Enum):
    """Статус объекта в памяти"""
    ALIVE = "alive"           # Живой объект
    MARKED = "marked"         # Помечен для удаления (M&S)
    DELETED = "deleted"       # Удалён
    LEAKED = "leaked"         # Утечка (RC)


class ReferenceStatus(Enum):
    """Статус ссылки"""
    ACTIVE = "active"         # Активная ссылка
    REMOVED = "removed"       # Удалённая ссылка
    CYCLE = "cycle"           # Часть цикла


@dataclass
class MemoryObject:
    """Представление объекта в памяти"""
    id: int
    size: int
    status: ObjectStatus
    ref_count: Optional[int] = None  # Для RC
    is_marked: Optional[bool] = None  # Для M&S
    is_root: bool = False
    
    def to_dict(self) -> Dict:
        data = asdict(self)
        data['status'] = self.status.value
        return data


@dataclass
class Reference:
    """Представление ссылки между объектами"""
    from_id: int
    to_id: int
    status: ReferenceStatus
    
    def to_dict(self) -> Dict:
        return {
            'from_id': self.from_id,
            'to_id': self.to_id,
            'status': self.status.value
        }


@dataclass
class HeapSnapshot:
    """Снимок состояния кучи"""
    gc_type: str  # 'RC' или 'MS'
    objects: List[MemoryObject]
    references: List[Reference]
    
    def to_dict(self) -> Dict:
        return {
            'gc_type': self.gc_type,
            'objects': [obj.to_dict() for obj in self.objects],
            'references': [ref.to_dict() for ref in self.references]
        }


@dataclass
class MemoryStatistics:
    """Статистика использования памяти"""
    total_allocated: int      # Всего выделено
    total_freed: int          # Всего освобождено
    peak_memory: int          # Пиковое использование
    leaked_memory: int        # Утечка памяти
    recovery_percent: float   # Процент восстановления памяти
    objects_created: int      # Создано объектов
    objects_left: int         # Осталось объектов
    execution_time_ms: float  # Время выполнения в мс
    
    def to_dict(self) -> Dict:
        return asdict(self)


@dataclass
class SimulationState:
    """Полное состояние симуляции"""
    parameters: Dict  # Параметры симуляции
    rc_heap: HeapSnapshot
    ms_heap: HeapSnapshot
    rc_stats: MemoryStatistics
    ms_stats: MemoryStatistics
    
    def to_dict(self) -> Dict:
        return {
            'parameters': self.parameters,
            'rc_heap': self.rc_heap.to_dict(),
            'ms_heap': self.ms_heap.to_dict(),
            'rc_stats': self.rc_stats.to_dict(),
            'ms_stats': self.ms_stats.to_dict()
        }