const API_BASE = '/api';
let currentRCData = null;
let currentMSData = null;

document.addEventListener('DOMContentLoaded', function() {
  console.log('🚀 Initializing GC Visualizer');

  // Загружаем конфиг
  loadConfiguration();

  // Привязываем обработчики
  const simulateBtn = document.getElementById('simulate-btn');
  if (simulateBtn) {
    simulateBtn.addEventListener('click', runSimulation);
    console.log('Simulate button found and bound');
  } else {
    console.error('Simulate button NOT found!');
  }

  // История
  const historyBtn = document.getElementById('history-btn');
  if (historyBtn) {
    historyBtn.addEventListener('click', loadHistory);
    console.log('History button found and bound');
  }

  // Кнопки анимации RC и MS
  const rcAnimBtn = document.getElementById('rc-anim-btn');
  if (rcAnimBtn) {
    rcAnimBtn.addEventListener('click', () => animateRC());
    console.log('RC Animation button bound');
  } else {
    console.error('RC Animation button NOT found!');
  }

  const msAnimBtn = document.getElementById('ms-anim-btn');
  if (msAnimBtn) {
    msAnimBtn.addEventListener('click', () => animateMS());
    console.log('MS Animation button bound');
  } else {
    console.error('MS Animation button NOT found!');
  }

  console.log('Initialization complete\n');
});

async function loadConfiguration() {
  try {
    const response = await fetch(`${API_BASE}/config`);
    const config = await response.json();
    console.log('Configuration loaded:', config);

    // Заполняем селект сценариев
    const scenarioSelect = document.getElementById('scenario-select');
    if (scenarioSelect && config.scenarios) {
        scenarioSelect.innerHTML = '';
      Object.entries(config.scenarios).forEach(([key, name]) => {
        const option = document.createElement('option');
        option.value = key;
        option.textContent = name;
        scenarioSelect.appendChild(option);
      });
      console.log('Scenarios loaded into select');
    }

    // Устанавливаем значения по умолчанию
    if (config.defaults) {
      const heapInput = document.getElementById('heap-size');
      const numObjInput = document.getElementById('num-objects');
      const sizeInput = document.getElementById('object-size');

      if (heapInput) heapInput.value = config.defaults.heap_size / (1024 * 1024);
      if (numObjInput) numObjInput.value = config.defaults.num_objects;
      if (sizeInput) sizeInput.value = config.defaults.object_size;
      console.log('Default values set');
    }
  } catch (error) {
    console.error('Failed to load configuration:', error);
  }
}

async function runSimulation() {
  console.log('\n' + '='.repeat(70));
  console.log('Starting simulation...');
  console.log('='.repeat(70));

  try {
    // Получаем параметры из формы
    const heapSizeMB = parseInt(document.getElementById('heap-size').value);
    const numObjects = parseInt(document.getElementById('num-objects').value);
    const objectSize = parseInt(document.getElementById('object-size').value);
    const scenarioType = document.getElementById('scenario-select').value;

    // Валидация
    if (!heapSizeMB || !numObjects || !objectSize) {
      alert('Please fill all parameters');
      return;
    }

    // Конвертируем MB в bytes
    const heapSize = heapSizeMB * 1024 * 1024;

    console.log('Sending parameters:');
    console.log(` Heap Size: ${heapSizeMB} MB (${heapSize} bytes)`);
    console.log(` Objects: ${numObjects}`);
    console.log(` Object Size: ${objectSize} bytes`);
    console.log(` Scenario: ${scenarioType}`);

    // Показываем загрузку
    showLoadingState(true);

    // Отправляем запрос на бэкэнд
    const response = await fetch(`${API_BASE}/simulate`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        heap_size: heapSize,
        num_objects: numObjects,
        object_size: objectSize,
        scenario_type: scenarioType
      })
    });

    let result = await response.json();

    // Если result - строка, парсим её
    if (typeof result === 'string') {
      result = JSON.parse(result);
    }

    showLoadingState(false);

    // Проверяем ошибки
    if (result.error) {
      console.error('Simulation failed:', result.error);
      alert(`Error: ${result.error}`);
      return;
    }

    if (!result.rc || !result.ms) {
      console.error('Invalid response structure:', result);
      alert('Error: Invalid response from server');
      return;
    }

    console.log('Simulation completed successfully!');
    console.log('\nRC Result:');
    console.log(result.rc);
    console.log('\nMS Result:');
    console.log(result.ms);

    // Сохраняем результаты для последующей анимации
    currentRCData = result.rc;
    currentMSData = result.ms;

    // Обновляем статистику
    updateRCStatistics(result.rc);
    updateMSStatistics(result.ms);

    // Сравниваем результаты
    compareStatistics(result.rc, result.ms);

    console.log('='.repeat(70));
    alert('Simulation complete!\n\nNow click "Animate RC" or "Animate MS" to see the visualization!');

  } catch (error) {
    console.error('Simulation error:', error);
    alert(`Error: ${error.message}`);
    showLoadingState(false);
  }
}

async function animateRC() {
  if (!currentRCData) {
    alert('Please run simulation first!');
    return;
  }

  console.log('\n' + '='.repeat(70));
  console.log('ANIMATE RC BUTTON CLICKED');
  console.log('='.repeat(70));

  try {
    // Проверяем что данные есть
    console.log('Checking data...');
    console.log('currentRCData type:', typeof currentRCData);
    console.log('currentRCData:', currentRCData);

    // ПРЯМОЙ ВЫЗОВ animateOperations() с данными
    if (typeof rcVisualizer !== 'undefined') {
      console.log('\nrcVisualizer found');
      console.log('Calling rcVisualizer.animateOperations(currentRCData)...\n');

      // Передаем ВЕСЬ объект с objects и references!
      await rcVisualizer.animateOperations(currentRCData);

      console.log('\nRC Animation completed');
      console.log('='.repeat(70));
    } else {
      console.error('rcVisualizer not found!');
      alert('Error: Visualization module not loaded');
    }
  } catch (error) {
    console.error('Animation error:', error);
    console.error('Stack:', error.stack);
    alert(`Error: ${error.message}`);
  }
}

async function animateMS() {
  if (!currentMSData) {
    alert('Please run simulation first!');
    return;
  }

  console.log('\n' + '='.repeat(70));
  console.log(' ANIMATE MS BUTTON CLICKED');
  console.log('='.repeat(70));

  try {
    // Проверяем что данные есть
    console.log('🔍 Checking data...');
    console.log('currentMSData type:', typeof currentMSData);
    console.log('currentMSData:', currentMSData);

    // ПРЯМОЙ ВЫЗОВ animateOperations() с данными
    if (typeof msVisualizer !== 'undefined') {
      console.log('\nmsVisualizer found');
      console.log('Calling msVisualizer.animateOperations(currentMSData)...\n');

      // Передаем ВЕСЬ объект с objects и references!
      await msVisualizer.animateOperations(currentMSData);

      console.log('\nMS Animation completed');
      console.log('='.repeat(70));
    } else {
      console.error('msVisualizer not found!');
      alert('Error: Visualization module not loaded');
    }
  } catch (error) {
    console.error('Animation error:', error);
    console.error('Stack:', error.stack);
    alert(`Error: ${error.message}`);
  }
}

function updateRCStatistics(result) {
  if (!result || !result.stats) {
    console.warn('No RC stats');
    return;
  }

  console.log('Updating RC statistics:', result.stats);

  const stats = result.stats;
  const formatBytes = (bytes) => {
    if (bytes === 0) return '0 B';
    if (bytes < 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(Math.abs(bytes)) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const updateElement = (id, value) => {
    const el = document.getElementById(id);
    if (el) el.textContent = value;
  };

  updateElement('rc-total-allocated', formatBytes(stats.total_allocated || 0));
  updateElement('rc-total-freed', formatBytes(stats.total_freed || 0));
  updateElement('rc-leaked', formatBytes(stats.leaked_memory || 0));
  updateElement('rc-objects-left', (stats.objects_left || 0).toString());
  updateElement('rc-exec-time', (stats.execution_time_ms || 0).toFixed(2) + ' ms');
  updateElement('rc-recovery', ((stats.recovery_percent || 0).toFixed(1)) + '%');

  console.log('RC stats updated');
}


function updateMSStatistics(result) {
  if (!result || !result.stats) {
    console.warn('No MS stats');
    return;
  }

  console.log('Updating MS statistics:', result.stats);

  const stats = result.stats;
  const formatBytes = (bytes) => {
    if (bytes === 0) return '0 B';
    if (bytes < 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(Math.abs(bytes)) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const updateElement = (id, value) => {
    const el = document.getElementById(id);
    if (el) el.textContent = value;
  };

  updateElement('ms-total-allocated', formatBytes(stats.total_allocated || 0));
  updateElement('ms-total-freed', formatBytes(stats.total_freed || 0));
  updateElement('ms-leaked', formatBytes(stats.leaked_memory || 0));
  updateElement('ms-objects-left', (stats.objects_left || 0).toString());
  updateElement('ms-exec-time', (stats.execution_time_ms || 0).toFixed(2) + ' ms');
  updateElement('ms-recovery', ((stats.recovery_percent || 0).toFixed(1)) + '%');

  console.log('MS stats updated');
}

function compareStatistics(rc, ms) {
  if (!rc || !ms || !rc.stats || !ms.stats) {
    console.warn('Missing data for comparison');
    return;
  }

  const rcStats = rc.stats;
  const msStats = ms.stats;

  console.log('\nCOMPARISON RESULTS:');
  console.log('─'.repeat(50));
  console.log('Reference Counting:');
  console.log(` Objects created: ${rcStats.objects_created}`);
  console.log(` Objects left: ${rcStats.objects_left}`);
  console.log(` Leaked: ${rcStats.leaked_memory} bytes`);
  console.log(` Time: ${rcStats.execution_time_ms?.toFixed(2)} ms`);
  console.log(` Recovery: ${rcStats.recovery_percent?.toFixed(1)}%`);

  console.log('\nMark & Sweep:');
  console.log(` Objects created: ${msStats.objects_created}`);
  console.log(` Objects left: ${msStats.objects_left}`);
  console.log(` Leaked: ${msStats.leaked_memory} bytes`);
  console.log(` Time: ${msStats.execution_time_ms?.toFixed(2)} ms`);
  console.log(` Recovery: ${msStats.recovery_percent?.toFixed(1)}%`);

  console.log('─'.repeat(50));
}

async function loadHistory() {
  try {
    const response = await fetch(`${API_BASE}/history`);
    const data = await response.json();

    console.log('History loaded:', data);

    if (data.success && data.history && data.history.length > 0) {
      const historyList = data.history.map((sim, idx) => {
        const params = sim.parameters;
        return `${idx + 1}. Scenario: ${params.scenario_type}, Objects: ${params.num_objects}`;
      }).join('\n');

      alert(`Total simulations: ${data.total}\n\n${historyList}`);
    } else {
      alert('No history yet');
    }
  } catch (error) {
    console.error('Failed to load history:', error);
  }
}

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ

function showLoadingState(isLoading) {
  const btn = document.getElementById('simulate-btn');
  if (btn) {
    if (isLoading) {
      btn.disabled = true;
      btn.textContent = 'Running simulation...';
    } else {
      btn.disabled = false;
      btn.textContent = 'Run Simulation';
    }
  }
}

console.log('main.js loaded');