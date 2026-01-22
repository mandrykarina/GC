#  Garbage Collector Visualization

> **Интерактивная визуализация и сравнение двух основных алгоритмов сборки мусора**

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Python](https://img.shields.io/badge/Python-3.8%2B-green?style=flat-square)](https://www.python.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

## О проекте

Это **комплексная система** для изучения и визуализации работы двух основных алгоритмов сборки мусора:

- **Reference Counting (RC)** — подсчёт ссылок в реальном времени
- **Mark & Sweep (MS)** — двухфазный алгоритм с маркировкой и очисткой

### Ключевые возможности

**Интерактивная визуализация**
- Реальная анимация работы GC в браузере
- Диаграммы графа объектов с цветовой кодировкой
- Пошаговое воспроизведение каждой операции

**Несколько сценариев**
- `basic` — базовое выделение и удаление
- `cycle_leak` — демонстрация циклических ссылок (слабость RC!)

**Сравнительный анализ**
- Время выполнения для каждого алгоритма
- Объём утёкшей памяти
- Количество операций
- Детальные статистики

---

## Архитектура

```
GC/
├──  src/                          # C++ ядро (два сборщика мусора)
│   ├── reference_counting/          # Reference Counting реализация
│   │   ├── include/                 # Заголовочные файлы
│   │   └── src/                     # Исходный код RC
│   └── mark_sweep/                  # Mark & Sweep реализация
│       ├── include/                 # Заголовочные файлы
│       └── src/                     # Исходный код MS
│
├──  visualization/                # Flask веб-приложение
│   ├── app.py                       # Главная Flask app
│   ├── config.py                    # Конфигурация
│   ├── core/
│   │   ├── gc_simulator.py          # Симулятор GC
│   │   └── data_models.py           # Модели данных
│   ├── routes/
│   │   └── api.py                   # REST API endpoints
│   ├── templates/
│   │   └── index.html               # Главная страница
│   └── static/
│       ├── js/
│       │   ├── main.js              # Основная логика
│       │   ├── visualization.js     # D3.js визуализация 
│       │   └── statistics.js        # Статистика
│       └── css/
│           └── style.css            # Стили
│
├── CMakeLists.txt                   # Build конфиг
├── Readme.MD                        # Документация
└── .gitignore                       # Git игнор файлы
```

---

## Быстрый старт

### Требования

- **C++ 17** компилятор (MSVC, GCC, Clang)
- **CMake 3.10+**
- **Python 3.8+**
- **Flask** и зависимости

### Установка и запуск

#### Клонируй репозиторий
```bash
git clone https://github.com/mandrykarina/GC.git
cd GC
```

#### Собери C++ часть
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

#### Установи Python зависимости
```bash
pip install -r visualization/requirements.txt
```

#### Запусти Flask приложение
```bash
cd visualization
python app.py
```

#### Открой в браузере
```
http://localhost:5000
```

---

## Примеры использования

### Через веб-интерфейс

1. Выбери **параметры симуляции**:
   - Размер heap (32-512 MB)
   - Количество объектов (5-100)
   - Размер объекта (64-1024 bytes)
   - Сценарий (basic, cascade_delete, cycle_leak)

2. Нажми **"RUN SIMULATION"**

3. Смотри **анимацию** двух алгоритмов рядом:
   - Левая панель: Reference Counting
   - Правая панель: Mark & Sweep

### Через командную строку

```bash
# Запусти только симулятор
./build/bin/Release/gc_unified.exe 2 100 64 32

# Параметры: <algorithm> <objects> <size> <heap_mb>
# algorithm: 1=RC, 2=MS
# objects: количество объектов
# size: размер каждого объекта
# heap_mb: размер памяти
```

---

## Основные возможности

### Reference Counting (RC)
```
Детерминированное удаление (сразу при ref_count=0)
Низкие паузы
Не может удалить циклические ссылки
Overhead на каждый increment/decrement
```

### Mark & Sweep (MS)
```
Удаляет циклические ссылки
Нет overhead на каждый assign
Stop-the-world pauses
Фрагментация памяти
```

---

## Цветовая кодировка в визуализации

| Цвет | Статус | Значение |
|------|--------|----------|
|  Красный | `leaked` | Объект утекает (RC на цикле) |
|  Зелёный | `alive` | Объект живой, в памяти |
|  Серый | `deleted` | Объект удалён |
|  Голубой | `root` | Корневой объект |

---

##  Статистика

После каждой симуляции ты видишь:

- **Memory Allocated** — выделено памяти
- **Memory Leaked** — утекло памяти
- **Objects Created** — создано объектов
- **Objects Deleted** — удалено объектов
- **Execution Time** — время выполнения (мс)

---

##  Сценарии симуляции

### `basic`
Базовый сценарий: выделение → связывание → удаление
```
Root → Obj1 → Obj2 → Obj3

```

### `cycle_leak` 
**Циклические ссылки** — самый интересный сценарий!
```
Root → Obj0 → Obj1 → Obj2 → ... → ObjN → Obj0
            ↑_____________________________↑ ЦИКЛ!

Результат:
- RC:  ВСЕ КРАСНЫЕ (утечка!)
- MS:  ВСЕ СЕРЫЕ (удаляет успешно)
```

---

##  Разработка

### Структура кода

**C++ Core:**
- `HeapObject` — базовый объект в памяти
- `RCHeap` — Reference Counting сборщик
- `MarkSweepGC` — Mark & Sweep сборщик

**Python Backend:**
- `GCSimulator` — запускает оба алгоритма
- `api.py` — REST endpoints
- `data_models.py` — JSON сериализация

**Frontend:**
- D3.js для анимации графов
- WebSocket для real-time обновлений
- Bootstrap для UI

### Добавить новый сценарий

Добавь JSON файл в `reference_counting/scenarios/` и `mark_sweep/scenarios/`:

```json
{
  "name": "my_scenario",
  "objects": [
    { "id": 0, "size": 64, "refs": [1, 2] },
    { "id": 1, "size": 64, "refs": [2] },
    { "id": 2, "size": 64, "refs": [] }
  ]
}
```

---

##  Документация

- **[Теория GC](docs/gc-theory.md)** — базовые концепции
- **[API Reference](docs/api.md)** — REST endpoints
- **[Architecture](docs/architecture.md)** — внутреннее устройство

---

##  Known Issues

- [ ] Большие графы (>1000 объектов) могут быть медленными в браузере
- [ ] Memory leak на циклических ссылках в RC (это фича, не баг!)
- [ ] Windows only (тестировано на MSVC)




**Made with ❤️ using C++17, Python, and D3.js**

*Последнее обновление: January 2026*
