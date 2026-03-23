#!/usr/bin/env python3
"""
Улучшенный планировщик для запуска 2.py каждые 5 минут
с обработкой ошибок и возможностью повтора
"""

import time
import subprocess
import os
from datetime import datetime

# Конфигурация
INTERVAL_MINUTES = 10  # Интервал запуска в минутах
RETRY_ON_ERROR = False  # Повторять ли при ошибке
RETRY_DELAY_MINUTES = 1  # Задержка перед повтором (если включено)
MAX_RETRIES = 5  # Максимум попыток повтора

def run_script():
    """Запускает скрипт 2.py"""
    current_dir = os.path.dirname(os.path.abspath(__file__))
    script_path = os.path.join(current_dir, "script2.py")
    
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Запуск 2.py...")
    
    try:
        result = subprocess.run(
            ["python3", script_path],
            capture_output=True,
            text=True,
            cwd=current_dir,
            timeout=300  # Таймаут 5 минут
        )
        
        # Выводим результат
        if result.stdout:
            print(f"Вывод:\n{result.stdout}")
        if result.stderr:
            # Показываем только последнюю строку ошибки для краткости
            error_lines = result.stderr.strip().split('\n')
            last_error = error_lines[-1] if error_lines else result.stderr
            print(f"Ошибка: {last_error}")
        
        # Проверяем код возврата
        if result.returncode == 0:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✓ Успешно завершено\n")
            return True
        else:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Завершено с ошибкой (код: {result.returncode})\n")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Превышен таймаут выполнения\n")
        return False
    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Ошибка при запуске: {e}\n")
        return False

def run_with_retry():
    """Запускает скрипт с возможностью повтора при ошибке"""
    if not RETRY_ON_ERROR:
        run_script()
        return
    
    for attempt in range(1, MAX_RETRIES + 1):
        success = run_script()
        
        if success:
            break
            
        if attempt < MAX_RETRIES:
            print(f"Попытка {attempt}/{MAX_RETRIES} не удалась. "
                  f"Повтор через {RETRY_DELAY_MINUTES} мин...\n")
            time.sleep(RETRY_DELAY_MINUTES * 60)
        else:
            print(f"Все {MAX_RETRIES} попытки исчерпаны.\n")

print("=" * 60)
print("Планировщик запуска 2.py")
print("=" * 60)
print(f"Интервал запуска: каждые {INTERVAL_MINUTES} минут")
print(f"Повтор при ошибке: {'Да' if RETRY_ON_ERROR else 'Нет'}")
if RETRY_ON_ERROR:
    print(f"Задержка повтора: {RETRY_DELAY_MINUTES} мин")
    print(f"Максимум попыток: {MAX_RETRIES}")
print("Нажмите Ctrl+C для остановки.")
print("=" * 60 + "\n")

# Основной цикл
try:
    while True:
        run_with_retry()
        
        next_run = datetime.now().timestamp() + (INTERVAL_MINUTES * 60)
        next_run_str = datetime.fromtimestamp(next_run).strftime('%Y-%m-%d %H:%M:%S')
        
        print(f"Следующий запуск: {next_run_str}")
        print(f"Ожидание {INTERVAL_MINUTES} минут...\n")
        
        time.sleep(INTERVAL_MINUTES * 60)
        
except KeyboardInterrupt:
    print("\n" + "=" * 60)
    print("Планировщик остановлен.")
    print("=" * 60)
