/**
 * ✅ STATISTICS.JS - Обновление статистики от C++
 * Парсит реальные метрики от сборщиков мусора
 */

// ============================================
// ФОРМАТИРОВАНИЕ
// ============================================

function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    if (bytes < 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(Math.abs(bytes)) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

function formatTime(ms) {
    if (typeof ms !== 'number' || ms < 0) return '0 ms';
    if (ms < 1) return (ms * 1000).toFixed(0) + ' µs';
    return ms.toFixed(2) + ' ms';
}

function formatPercent(value) {
    if (typeof value !== 'number' || isNaN(value)) return '0%';
    return value.toFixed(1) + '%';
}

// ============================================
// ОБНОВЛЕНИЕ RC СТАТИСТИКИ
// ============================================

/**
 * ✅ Обновляет RC статистику из реальных данных C++
 */
function updateRCStatistics(rcData) {
    console.log('📊 Updating RC Statistics:', rcData);
    
    if (!rcData || !rcData.stats) {
        console.warn('No RC stats available');
        resetRCStatistics();
        return;
    }
    
    const stats = rcData.stats;
    
    // Обновляем элементы на странице
    updateElement('rc-total-allocated', formatBytes(stats.total_allocated || 0));
    updateElement('rc-total-freed', formatBytes(stats.total_freed || 0));
    updateElement('rc-leaked', formatBytes(stats.leaked_memory || 0));
    updateElement('rc-objects-left', (stats.objects_left || 0).toString());
    updateElement('rc-exec-time', formatTime(stats.execution_time_ms || 0));
    updateElement('rc-recovery', formatPercent(stats.recovery_percent || 0));
    
    console.log(`✅ RC Statistics:`);
    console.log(`   Created: ${stats.objects_created}`);
    console.log(`   Left: ${stats.objects_left}`);
    console.log(`   Leaked: ${formatBytes(stats.leaked_memory || 0)}`);
    console.log(`   Recovery: ${formatPercent(stats.recovery_percent || 0)}`);
    console.log(`   Time: ${formatTime(stats.execution_time_ms || 0)}`);
}

// ============================================
// ОБНОВЛЕНИЕ MS СТАТИСТИКИ
// ============================================

/**
 * ✅ Обновляет MS статистику из реальных данных C++
 */
function updateMSStatistics(msData) {
    console.log('📊 Updating MS Statistics:', msData);
    
    if (!msData || !msData.stats) {
        console.warn('No MS stats available');
        resetMSStatistics();
        return;
    }
    
    const stats = msData.stats;
    
    // Обновляем элементы на странице
    updateElement('ms-total-allocated', formatBytes(stats.total_allocated || 0));
    updateElement('ms-total-freed', formatBytes(stats.total_freed || 0));
    updateElement('ms-leaked', formatBytes(stats.leaked_memory || 0));
    updateElement('ms-objects-left', (stats.objects_left || 0).toString());
    updateElement('ms-exec-time', formatTime(stats.execution_time_ms || 0));
    updateElement('ms-recovery', formatPercent(stats.recovery_percent || 0));
    
    console.log(`✅ MS Statistics:`);
    console.log(`   Created: ${stats.objects_created}`);
    console.log(`   Left: ${stats.objects_left}`);
    console.log(`   Leaked: ${formatBytes(stats.leaked_memory || 0)}`);
    console.log(`   Recovery: ${formatPercent(stats.recovery_percent || 0)}`);
    console.log(`   Time: ${formatTime(stats.execution_time_ms || 0)}`);
}

// ============================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================

/**
 * Безопасно обновляет текст элемента
 */
function updateElement(elementId, value) {
    const element = document.getElementById(elementId);
    if (element) {
        element.textContent = value;
    } else {
        console.warn(`Element ${elementId} not found`);
    }
}

/**
 * Сброс RC статистики
 */
function resetRCStatistics() {
    updateElement('rc-total-allocated', '0 B');
    updateElement('rc-total-freed', '0 B');
    updateElement('rc-leaked', '0 B');
    updateElement('rc-objects-left', '0');
    updateElement('rc-exec-time', '0 ms');
    updateElement('rc-recovery', '0%');
}

/**
 * Сброс MS статистики
 */
function resetMSStatistics() {
    updateElement('ms-total-allocated', '0 B');
    updateElement('ms-total-freed', '0 B');
    updateElement('ms-leaked', '0 B');
    updateElement('ms-objects-left', '0');
    updateElement('ms-exec-time', '0 ms');
    updateElement('ms-recovery', '0%');
}

// ============================================
// СРАВНЕНИЕ RC vs MS
// ============================================

/**
 * ✅ Сравнивает результаты RC vs MS и выводит выводы
 */
function compareStatistics(rcData, msData) {
    console.log('🔄 Comparing RC vs MS');
    
    if (!rcData || !msData || !rcData.stats || !msData.stats) {
        console.warn('Missing data for comparison');
        return;
    }
    
    const rc = rcData.stats;
    const ms = msData.stats;
    
    console.log('📈 COMPARISON RESULTS:');
    console.log('─'.repeat(50));
    console.log(`Reference Counting:`);
    console.log(`  Objects created: ${rc.objects_created}`);
    console.log(`  Objects left: ${rc.objects_left}`);
    console.log(`  Leaked: ${formatBytes(rc.leaked_memory || 0)}`);
    console.log(`  Time: ${formatTime(rc.execution_time_ms || 0)}`);
    console.log(`  Recovery: ${formatPercent(rc.recovery_percent || 0)}`);
    
    console.log(`\nMark & Sweep:`);
    console.log(`  Objects created: ${ms.objects_created}`);
    console.log(`  Objects left: ${ms.objects_left}`);
    console.log(`  Leaked: ${formatBytes(ms.leaked_memory || 0)}`);
    console.log(`  Time: ${formatTime(ms.execution_time_ms || 0)}`);
    console.log(`  Recovery: ${formatPercent(ms.recovery_percent || 0)}`);
    
    // Анализ результатов
    console.log(`\n📊 ANALYSIS:`);
    
    const rcLeaked = rc.leaked_memory || 0;
    const msLeaked = ms.leaked_memory || 0;
    const leakDiff = rcLeaked - msLeaked;
    
    if (leakDiff > 0) {
        console.log(`  ✅ MS is BETTER: ${formatBytes(leakDiff)} less leak`);
        console.log(`     RC detected cycles (Reference Counting limitation)`);
    } else if (leakDiff < 0) {
        console.log(`  ⚠️  RC is better: ${formatBytes(Math.abs(leakDiff))} less leak`);
    } else {
        console.log(`  ≈️  Same result: Both recovered equally`);
    }
    
    const timeDiff = (rc.execution_time_ms || 0) - (ms.execution_time_ms || 0);
    if (timeDiff > 0) {
        console.log(`  🚀 MS is faster: ${formatTime(timeDiff)}`);
    } else if (timeDiff < 0) {
        console.log(`  🚀 RC is faster: ${formatTime(Math.abs(timeDiff))}`);
    }
    
    console.log('─'.repeat(50));
}

console.log('✓ statistics.js loaded');