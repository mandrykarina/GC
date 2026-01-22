/**
 * ✅✅✅ VISUALIZATION.JS - С ПРАВИЛЬНЫМ ОТОБРАЖЕНИЕМ ЦИКЛИЧЕСКИХ ССЫЛОК!
 */

const ANIMATION_DELAY = 400;

class GCVisualizer {
  constructor(svgSelector) {
    this.svgSelector = svgSelector;
    this.svg = d3.select(svgSelector);
    this.nodes = [];
    this.links = [];
    this.simulation = null;
    this.width = 0;
    this.height = 0;
    this.isAnimating = false;
    this.currentScenario = 'basic';
    this.gcType = svgSelector === '#rc-graph' ? 'RC' : 'MS';
    console.log(`🎨 GCVisualizer created for ${svgSelector} (${this.gcType})`);
    this.setupSVG();
  }

  setupSVG() {
    const container = document.querySelector(this.svgSelector)?.parentElement;
    if (!container) {
      console.error(`❌ Container for ${this.svgSelector} not found!`);
      return;
    }

    const width = container.offsetWidth || 800;
    const height = 500;
    this.width = width;
    this.height = height;

    console.log(`📐 SVG setup: ${width}x${height}`);

    this.svg.attr('width', width).attr('height', height);
    this.svg.selectAll('*').remove();

    this.linkGroup = this.svg.append('g').attr('class', 'links');
    this.nodeGroup = this.svg.append('g').attr('class', 'nodes');
    this.labelGroup = this.svg.append('g').attr('class', 'labels');

    const defs = this.svg.append('defs');
    
    // Стрелка для обычных ссылок (зеленая)
    defs.append('marker')
      .attr('id', `arrowhead-${this.svgSelector}`)
      .attr('markerWidth', 10)
      .attr('markerHeight', 10)
      .attr('refX', 24)
      .attr('refY', 3)
      .attr('orient', 'auto')
      .append('polygon')
      .attr('points', '0 0, 10 3, 0 6')
      .attr('fill', '#4ecdc4');

    // Стрелка для циклических ссылок (ярко-красная, БОЛЬШАЯ)
    defs.append('marker')
      .attr('id', `arrowhead-cycle-${this.svgSelector}`)
      .attr('markerWidth', 16)  // БОЛЬШЕ обычной
      .attr('markerHeight', 16)
      .attr('refX', 32)        // БОЛЬШЕ обычной
      .attr('refY', 5)
      .attr('orient', 'auto')
      .append('polygon')
      .attr('points', '0 0, 16 5, 0 10')
      .attr('fill', '#ff0000')
      .attr('stroke', '#cc0000')
      .attr('stroke-width', 2);

    // Стрелка для удаленных ссылок (серая)
    defs.append('marker')
      .attr('id', `arrowhead-removed-${this.svgSelector}`)
      .attr('markerWidth', 8)
      .attr('markerHeight', 8)
      .attr('refX', 20)
      .attr('refY', 3)
      .attr('orient', 'auto')
      .append('polygon')
      .attr('points', '0 0, 8 3, 0 6')
      .attr('fill', '#999999')
      .attr('opacity', 0.5);

    const zoom = d3.zoom().on('zoom', (event) => {
      this.svg.selectAll('g.links, g.nodes, g.labels').attr('transform', event.transform);
    });
    this.svg.call(zoom);
    console.log(`✓ SVG initialized`);
  }

  initSimulation() {
    if (this.simulation) {
      this.simulation.stop();
    }

    this.simulation = d3.forceSimulation(this.nodes)
    .force('link', d3.forceLink(this.links)
        .id(d => d.id)
        .distance(100)
        .strength(1.0))           // ← Изменил 0.8 → 1.0
    .force('charge', d3.forceManyBody().strength(-120))  // ← Изменил -250 → -120
    .force('center', d3.forceCenter(this.width / 2, this.height / 2).strength(1.2))  // ← Изменил 0.5 → 1.2


    this.simulation.on('tick', () => this.draw());
  }

  clear() {
    this.nodes = [];
    this.links = [];
    this.nodeGroup.selectAll('*').remove();
    this.linkGroup.selectAll('*').remove();
    this.labelGroup.selectAll('*').remove();
    if (this.simulation) {
      this.simulation.stop();
    }
    console.log(`🧹 Cleared ${this.gcType} visualizer`);
  }

  draw() {
    // Оптимизация: проверяем, нужно ли обновлять элементы
    if (this.nodes.length === 0 && this.links.length === 0) {
        return;
    }

    // ОБНОВЛЯЕМ СТРЕЛКИ
    const linkSelection = this.linkGroup.selectAll('line')
        .data(this.links, d => `${d.source.id}-${d.target.id}-${d.linkType || 'normal'}`);

    // Удаляем старые ссылки
    linkSelection.exit()
        .transition()
        .duration(200)
        .attr('opacity', 0)
        .remove();

    // Добавляем новые ссылки
    const linksEnter = linkSelection.enter()
        .append('line')
        .attr('stroke-width', d => d.linkType === 'cycle' ? 5 : 2) // ТОЛЩЕ для цикла
        .attr('stroke-dasharray', d => {
            if (d.status === 'removed') return '5,5';
            if (d.linkType === 'cycle') return '3,3'; // Пунктир для цикла
            return 'none';
        })
        .attr('stroke', d => {
            if (d.status === 'removed') return '#999999';
            if (d.linkType === 'cycle') return '#ff0000'; // 🔴 Ярко-красный для цикла
            return '#4ecdc4'; // 🟢 Зеленый для обычных
        })
        .attr('marker-end', d => {
            if (d.status === 'removed') return `url(#arrowhead-removed-${this.svgSelector})`;
            if (d.linkType === 'cycle') return `url(#arrowhead-cycle-${this.svgSelector})`; // 🔴 Циклическая стрелка
            return `url(#arrowhead-${this.svgSelector})`;
        })
        .attr('opacity', 0);

    // Объединяем и обновляем все ссылки
    const allLinks = linksEnter.merge(linkSelection)
        .attr('x1', d => {
            if (typeof d.source === 'object') return d.source.x || 0;
            return 0;
        })
        .attr('y1', d => {
            if (typeof d.source === 'object') return d.source.y || 0;
            return 0;
        })
        .attr('x2', d => {
            if (typeof d.target === 'object') return d.target.x || 0;
            return 0;
        })
        .attr('y2', d => {
            if (typeof d.target === 'object') return d.target.y || 0;
            return 0;
        })
        .attr('opacity', d => d.status === 'removed' ? 0.4 : 1)
        .attr('stroke-width', d => d.linkType === 'cycle' ? 5 : 2); // Обновляем толщину

    // Плавное появление новых ссылок
    linksEnter.transition()
        .duration(300)
        .attr('opacity', d => d.status === 'removed' ? 0.4 : 1);

    // ✅ ОТРИСОВЫВАЕМ МЕТКИ ДЛЯ ЦИКЛИЧЕСКИХ ССЫЛОК
    this.drawCycleLabels();

    // ОБНОВЛЯЕМ КРУЖКИ
    const nodeSelection = this.nodeGroup.selectAll('circle')
        .data(this.nodes, d => d.id);

    // Удаляем старые узлы
    nodeSelection.exit()
        .transition()
        .duration(200)
        .attr('r', 0)
        .attr('opacity', 0)
        .remove();

    // Добавляем новые узлы
    const nodesEnter = nodeSelection.enter()
        .append('circle')
        .attr('r', 0) // Начинаем с 0 для анимации
        .attr('stroke', '#fff')
        .attr('stroke-width', 3)
        .attr('fill', d => {
            // ✅ КОРЕНЬ - ЯРКО-КРАСНЫЙ ДЛЯ ВСЕХ СЦЕНАРИЕВ
            if (d.isRoot) return '#ff0000';
            if (d.status === 'leaked') return '#ff4444'; // 🔥 LEAKED (только для RC в цикле)
            if (d.status === 'deleted') return '#999999'; // ⚫ DELETED
            if (d.isMarked === false) return '#aaaaaa';  // ⚫ UNREACHABLE (только для MS)
            return '#4ecdc4';                    // 🟢 ALIVE
        })
        .attr('opacity', 0.8)
        .call(d3.drag()
            .on('start', (event, d) => {
                if (!event.active) this.simulation.alphaTarget(0.3).restart();
                d.fx = d.x;
                d.fy = d.y;
            })
            .on('drag', (event, d) => {
                d.fx = event.x;
                d.fy = event.y;
            })
            .on('end', (event, d) => {
                if (!event.active) this.simulation.alphaTarget(0);
                d.fx = null;
                d.fy = null;
            })
        );

    // Анимация появления новых узлов
    nodesEnter.transition()
        .duration(300)
        .attr('r', 30);

    // Объединяем и обновляем все узлы
    const allNodes = nodesEnter.merge(nodeSelection)
        .attr('cx', d => d.x || 0)
        .attr('cy', d => d.y || 0)
        .attr('fill', d => {
            // Обновляем цвет при изменении статуса
            if (d.isRoot) return '#ff0000';
            if (d.status === 'leaked') return '#ff4444';
            if (d.status === 'deleted') return '#999999';
            if (d.isMarked === false) return '#aaaaaa';
            return '#4ecdc4';
        })
        .attr('stroke', d => {
            if (d.isRoot) return '#ff3333'; // 🔴 Красная обводка для корня
            return '#ffffff';
        });

    // ОБНОВЛЯЕМ ТЕКСТ
    const textSelection = this.nodeGroup.selectAll('text')
        .data(this.nodes, d => d.id);

    // Удаляем старый текст
    textSelection.exit()
        .transition()
        .duration(200)
        .attr('opacity', 0)
        .remove();

    // Добавляем новый текст
    const textsEnter = textSelection.enter()
        .append('text')
        .attr('text-anchor', 'middle')
        .attr('dy', '0.3em')
        .attr('font-weight', 'bold')
        .attr('font-size', '14px')
        .attr('fill', d => {
            if (d.isRoot || d.status === 'leaked') return '#fff';
            if (d.status === 'deleted' || d.isMarked === false) return '#666';
            return '#fff';
        })
        .text(d => {
            const match = String(d.id).match(/\d+/);
            let text = match ? match[0] : d.id;
            // ✅ Добавляем R для корня
            if (d.isRoot) {
                text = `R${text}`;
            }
            return text;
        })
        .attr('opacity', 0);

    // Анимация появления текста
    textsEnter.transition()
        .duration(300)
        .attr('opacity', 1);

    // Объединяем и обновляем весь текст
    textsEnter.merge(textSelection)
        .attr('x', d => d.x || 0)
        .attr('y', d => d.y || 0)
        .attr('fill', d => {
            // Обновляем цвет текста при изменении статуса
            if (d.isRoot || d.status === 'leaked') return '#fff';
            if (d.status === 'deleted' || d.isMarked === false) return '#666';
            return '#fff';
        })
        .text(d => {
            const match = String(d.id).match(/\d+/);
            let text = match ? match[0] : d.id;
            if (d.isRoot) {
                text = `R${text}`;
            }
            return text;
        });
  }

  // ✅ НОВЫЙ МЕТОД: Отрисовка меток для циклических ссылок
  drawCycleLabels() {
    // Находим циклические ссылки
    const cycleLinks = this.links.filter(d => d.linkType === 'cycle' && d.status !== 'removed');
    
    // Обновляем метки
    const labelSelection = this.labelGroup.selectAll('text.cycle-label')
        .data(cycleLinks, d => `${d.source.id}-${d.target.id}-cycle`);
    
    // Удаляем старые метки
    labelSelection.exit()
        .transition()
        .duration(200)
        .attr('opacity', 0)
        .remove();
    
    // Добавляем новые метки
    const labelsEnter = labelSelection.enter()
        .append('text')
        .attr('class', 'cycle-label')
        .attr('text-anchor', 'middle')
        .attr('dy', '-0.5em')
        .attr('font-size', '11px')
        .attr('font-weight', 'bold')
        .attr('fill', '#ff0000')
        .attr('stroke', '#fff')
        .attr('stroke-width', '2px')
        .attr('paint-order', 'stroke')
        .text('CYCLE')
        .attr('opacity', 0);
    
    // Обновляем позиции всех меток
    labelsEnter.merge(labelSelection)
        .attr('x', d => {
            const midX = (d.source.x + d.target.x) / 2;
            return midX;
        })
        .attr('y', d => {
            const midY = (d.source.y + d.target.y) / 2;
            // Смещаем немного выше линии
            const dx = d.target.x - d.source.x;
            const dy = d.target.y - d.source.y;
            const angle = Math.atan2(dy, dx);
            const offset = 15;
            return midY - Math.sin(angle) * offset;
        })
        .attr('opacity', 1);
    
    // Анимация появления
    labelsEnter.transition()
        .duration(300)
        .attr('opacity', 1);
  }

  async animateOperations(data) {
    console.log('\n' + '='.repeat(70));
    console.log(`🎬 Starting ${this.gcType} animation`);
    console.log(`Scenario: ${data.scenario || 'basic'}`);
    console.log('='.repeat(70));

    this.currentScenario = data.scenario || 'basic';
    
    let operations = [];

    if (Array.isArray(data)) {
      operations = data;
    } else if (data && data.phases && data.phases.length > 0) {
      console.log(`📋 Found ${data.phases.length} phases in data`);
      operations = this.phasesToOperations(data.phases);
      console.log(`✅ Converted phases to ${operations.length} operations`);
    } else if (data && data.objects) {
      console.log(`⚠️  No phases found, generating operations from data...`);
      operations = this.generateOperationsFromData(data);
    } else {
      console.error('❌ Invalid data');
      return;
    }

    if (!operations || operations.length === 0) {
      console.warn('⚠️ No operations');
      return;
    }

    console.log(`\n🎬 ${operations.length} operations total\n`);
    this.clear();
    this.initSimulation();
    this.isAnimating = true;

    for (let i = 0; i < operations.length; i++) {
      const op = operations[i];
      
      if (op.op === 'pause') {
        this.draw();
        await this.delay(ANIMATION_DELAY * 2);
        continue;
      }

      const objId = op.obj_id !== undefined ? op.obj_id : '?';
      console.log(`[${i + 1}/${operations.length}] ${op.op.toUpperCase().padEnd(15)} obj ${objId}`);

      switch (op.op) {
        case 'allocate':
          this.allocateObject(op);
          break;
        case 'addroot':
          this.addRootReference(op);
          break;
        case 'addref':
          this.addReference(op);
          break;
        case 'cycle_closure':
          this.addCycleClosure(op);
          break;
        case 'removeroot':
          this.removeRootReference(op);
          break;
        case 'removeref':
          this.removeReference(op);
          break;
        case 'mark_unreachable':
          this.markUnreachable(op);
          break;
        case 'mark_leaked':
          this.markLeaked(op);
          break;
        case 'delete':
          this.deleteObject(op);
          break;
        default:
          console.warn(`Unknown operation: ${op.op}`);
      }

      this.draw();

      await this.delay(ANIMATION_DELAY);
    }

    this.isAnimating = false;
    console.log(`\n✅ ${this.gcType} animation complete!`);
    console.log('='.repeat(70) + '\n');
  }

  /**
   * ✅ ГЕНЕРАЦИЯ ОПЕРАЦИЙ ИЗ ДАННЫХ JSON
   * Теперь MS тоже показывает циклический граф!
   */
  generateOperationsFromData(data) {
    const operations = [];
    const objectCount = data.objects ? data.objects.length : 20;
    const scenarioType = data.scenario || 'basic';
    const isCyclic = scenarioType && scenarioType.toLowerCase().includes('cycl');

    console.log(`\n📋 ${this.gcType} Simulation:`);
    console.log(`  Scenario: ${scenarioType} ${isCyclic ? '🌀' : '📏'}`);
    console.log(`  Objects: ${objectCount}`);
    console.log(`  GC Type: ${this.gcType}`);
    
    // Анализируем данные
    const aliveObjects = data.objects ? data.objects.filter(obj => obj.status !== 'deleted') : [];
    const deadObjects = data.objects ? data.objects.filter(obj => obj.status === 'deleted') : [];
    const leakedObjects = data.objects ? data.objects.filter(obj => obj.status === 'leaked') : [];
    
    console.log(`  Alive: ${aliveObjects.length}, Dead: ${deadObjects.length}, Leaked: ${leakedObjects.length}`);
    
    // ✅ ФАЗА 1: Выделение памяти
    console.log('\n📌 PHASE 1: Allocation');
    for (let i = 0; i < objectCount; i++) {
      operations.push({ 
        op: 'allocate', 
        obj_id: i, 
        size: 64 
      });
      if (i % 4 === 0) operations.push({ op: 'pause' });
    }
    console.log(`   ✓ Allocated ${objectCount} objects`);
    operations.push({ op: 'pause' });
    
    // ✅ ФАЗА 2: Создание графа
    console.log('\n📌 PHASE 2: Building graph');
    
    // ✅ ДОБАВЛЯЕМ КОРЕНЬ (объект 0) - ДЛЯ ВСЕХ СЦЕНАРИЕВ!
    operations.push({ op: 'addroot', obj_id: 0 });
    console.log(`   ✓ Object 0 is ROOT (🔴 RED)`);
    operations.push({ op: 'pause' });
    operations.push({ op: 'pause' });
    
    if (isCyclic) {
      // ✅ ЦИКЛИЧЕСКИЙ ГРАФ
      console.log(`   🌀 Creating CYCLIC graph...`);
      
      // Создаем линейную цепь
      for (let i = 1; i < objectCount; i++) {
        operations.push({ 
          op: 'addref', 
          obj_id: i - 1, 
          target_id: i 
        });
        if (i % 3 === 0) operations.push({ op: 'pause' });
      }
      
      operations.push({ op: 'pause' });
      
      // ✅ ЗАМЫКАЕМ ЦИКЛ! (ОСОБАЯ ОПЕРАЦИЯ)
      if (objectCount > 1) {
        operations.push({ 
          op: 'cycle_closure', 
          obj_id: objectCount - 1,
          target_id: 0,
          from: objectCount - 1,
          to: 0
        });
        console.log(`   🔴🔴🔴 CYCLE CLOSURE: ${objectCount - 1} → 0 🔴🔴🔴`);
        operations.push({ op: 'pause' });
        operations.push({ op: 'pause' });
      }
      
    } else {
      // ✅ ЛИНЕЙНАЯ ЦЕПЬ (BASIC)
      console.log(`   📏 Creating LINEAR chain...`);
      
      for (let i = 1; i < objectCount; i++) {
        operations.push({ 
          op: 'addref', 
          obj_id: i - 1, 
          target_id: i 
        });
        if (i % 4 === 0) operations.push({ op: 'pause' });
      }
    }
    
    operations.push({ op: 'pause' });
    operations.push({ op: 'pause' });
    
    // ✅ ФАЗА 3: Удаление графа
    console.log('\n📌 PHASE 3: Garbage collection');
    
    // Удаляем корень
    operations.push({ op: 'removeroot', obj_id: 0 });
    console.log(`   ✓ Removed root from object 0`);
    operations.push({ op: 'pause' });
    operations.push({ op: 'pause' });
    
    if (this.gcType === 'RC') {
      console.log(`   🧮 RC: Deleting when refcount = 0`);
      
      if (isCyclic) {
        // ✅ RC НА ЦИКЛЕ: УТЕЧКА!
        console.log(`   🔥 RC CANNOT HANDLE CYCLES!`);
        
        // Помечаем все объекты как leaked
        for (let i = 0; i < objectCount; i++) {
          if (leakedObjects.some(obj => obj.id === i)) {
            operations.push({ op: 'mark_leaked', obj_id: i });
            console.log(`   🔥 Object ${i} LEAKED (trapped in cycle)`);
            operations.push({ op: 'pause' });
          }
        }
        
      } else {
        // ✅ RC НА LINEAR: Каскадное удаление
        for (let i = 0; i < objectCount; i++) {
          if (deadObjects.some(obj => obj.id === i)) {
            // Удаляем ссылку на следующий объект
            if (i < objectCount - 1) {
              operations.push({ 
                op: 'removeref', 
                obj_id: i, 
                target_id: i + 1 
              });
              operations.push({ op: 'pause' });
            }
            
            operations.push({ op: 'delete', obj_id: i });
            console.log(`   🗑️  Object ${i} deleted (refcount=0)`);
            operations.push({ op: 'pause' });
          }
        }
      }
      
    } else {
      // ✅ MS НА ЛЮБОМ ГРАФЕ: Mark & Sweep
      console.log(`   🧹 MS: Mark & Sweep algorithm`);
      
      // Фаза Mark
      console.log(`   📍 MARK phase: Finding reachable objects`);
      operations.push({ op: 'pause' });
      
      for (let i = 0; i < objectCount; i++) {
        if (deadObjects.some(obj => obj.id === i)) {
          operations.push({ op: 'mark_unreachable', obj_id: i });
          if (i % 3 === 0) operations.push({ op: 'pause' });
        }
      }
      
      operations.push({ op: 'pause' });
      operations.push({ op: 'pause' });
      
      // Фаза Sweep
      console.log(`   🧹 SWEEP phase: Deleting unreachable objects`);
      operations.push({ op: 'pause' });
      
      for (let i = 0; i < objectCount; i++) {
        if (deadObjects.some(obj => obj.id === i)) {
          operations.push({ op: 'delete', obj_id: i });
          if (i % 2 === 0) operations.push({ op: 'pause' });
        }
      }
    }
    
    // Финальная пауза
    operations.push({ op: 'pause' });
    operations.push({ op: 'pause' });
    
    console.log(`\n✅ Generated ${operations.length} operations for ${this.gcType}`);
    
    return operations;
  }

  phasesToOperations(phases) {
    const operations = [];

    for (const phase of phases) {
      if (phase.name) {
        console.log(`\n📌 PHASE: ${phase.name}`);
        if (phase.description) {
          console.log(`   ${phase.description}`);
        }
      }

      if (phase.operations && Array.isArray(phase.operations)) {
        for (const op of phase.operations) {
          operations.push(op);
        }
      }
    }

    return operations;
  }

  // ============================================
  // ОПЕРАЦИИ С ОБЪЕКТАМИ
  // ============================================

  allocateObject(op) {
    const nodeId = op.obj_id;
    const node = {
      id: `obj_${nodeId}`,
      originalId: nodeId,
      isRoot: false,
      isMarked: true,
      status: 'alive',
      size: op.size || 64
    };
    this.nodes.push(node);
    this.simulation.nodes(this.nodes);
    this.simulation.alpha(1).restart();
  }

  addRootReference(op) {
    const nodeId = op.obj_id;
    const node = this.nodes.find(n => n.id === `obj_${nodeId}`);
    if (node) {
      node.isRoot = true;
      console.log(`      🔴 Object ${nodeId} is now ROOT`);
    }
  }

  removeRootReference(op) {
    const nodeId = op.obj_id;
    const node = this.nodes.find(n => n.id === `obj_${nodeId}`);
    if (node) {
      node.isRoot = false;
      console.log(`      ✓ Object ${nodeId} no longer ROOT`);
    }
  }

  markUnreachable(op) {
    const nodeId = op.obj_id;
    const node = this.nodes.find(n => n.id === `obj_${nodeId}`);
    if (node) {
      node.isMarked = false;
      console.log(`      ⚫ Object ${nodeId} MARKED as unreachable`);
    }
  }

  markLeaked(op) {
    const nodeId = op.obj_id;
    const node = this.nodes.find(n => n.id === `obj_${nodeId}`);
    if (node) {
      node.status = 'leaked';
      console.log(`      🔥 Object ${nodeId} LEAKED (cycle detected!)`);
    }
  }

  addReference(op) {
    const fromId = op.obj_id;
    const toId = op.target_id;

    const source = this.nodes.find(n => n.id === `obj_${fromId}`);
    const target = this.nodes.find(n => n.id === `obj_${toId}`);

    if (source && target) {
      const linkExists = this.links.some(
        l => l.source.id === source.id && l.target.id === target.id
      );

      if (!linkExists) {
        this.links.push({
          source: source,
          target: target,
          isRoot: false,
          linkType: 'normal',
          status: 'active'
        });

        this.simulation.force('link').links(this.links);
        this.simulation.alpha(1).restart();
        
        console.log(`      🟢 Added ref: ${fromId} → ${toId}`);
      }
    }
  }

  addCycleClosure(op) {
    const fromId = op.obj_id;
    const toId = op.target_id;

    const source = this.nodes.find(n => n.id === `obj_${fromId}`);
    const target = this.nodes.find(n => n.id === `obj_${toId}`);

    if (source && target) {
      // ✅ СОЗДАЕМ ЦИКЛИЧЕСКУЮ ССЫЛКУ (ОСОБЫЙ ТИП!)
      this.links.push({
        source: source,
        target: target,
        isRoot: false,
        linkType: 'cycle', // 🔴 ОСОБЫЙ ТИП!
        status: 'active',
        isCycleClosure: true
      });

      this.simulation.force('link').links(this.links);
      this.simulation.alpha(1).restart();
      
      console.log(`      🔴🔴🔴 CYCLE CLOSURE: ${fromId} → ${toId} 🔴🔴🔴`);
      console.log(`         ⚠️  REFERENCE CYCLE CREATED!`);
      
      if (this.gcType === 'RC') {
        console.log(`         🔥🔥🔥 RC WILL LEAK THESE OBJECTS!`);
      } else {
        console.log(`         ✅ MS CAN HANDLE CYCLES WITH MARK PHASE`);
      }
    }
  }

  removeReference(op) {
    const fromId = op.obj_id;
    const toId = op.target_id;

    const link = this.links.find(l => 
      l.source.id === `obj_${fromId}` && l.target.id === `obj_${toId}`
    );
    
    if (link) {
      link.status = 'removed';
      console.log(`      ✓ Removed ref: ${fromId} → ${toId}`);
    }
  }

  deleteObject(op) {
    const nodeId = op.obj_id;

    // Удаляем все ссылки связанные с этим объектом
    this.links = this.links.filter(l => {
      return l.source.id !== `obj_${nodeId}` && l.target.id !== `obj_${nodeId}`;
    });

    // Помечаем объект как удаленный
    const node = this.nodes.find(n => n.id === `obj_${nodeId}`);
    if (node) {
      node.status = 'deleted';
      node.isMarked = false;
      node.isRoot = false;
      console.log(`      🗑️  Object ${nodeId} DELETED`);
    }

    this.simulation.nodes(this.nodes);
    this.simulation.force('link').links(this.links);
  }

  delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
}

console.log('\n🚀 Initializing visualizers...');
const rcVisualizer = new GCVisualizer('#rc-graph');
const msVisualizer = new GCVisualizer('#ms-graph');
console.log('✅ visualization.js loaded!\n');

// Экспортируем для main.js
window.rcVisualizer = rcVisualizer;
window.msVisualizer = msVisualizer;
window.GCVisualizer = GCVisualizer;