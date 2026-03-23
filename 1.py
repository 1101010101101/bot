#!/usr/bin/env python3
import time
import subprocess
import os
from datetime import datetime

INTERVAL_MINUTES = 0
RETRY_ON_ERROR = False
RETRY_DELAY_MINUTES = 1
MAX_RETRIES = 5

def run_script():
    current_dir = os.path.dirname(os.path.abspath(__file__))

    # Удаляем куки перед каждым запуском
    cookies_file = os.path.join(current_dir, "cookies.pkl")
    if os.path.exists(cookies_file):
        os.remove(cookies_file)
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 🗑 Куки удалены")

    script_path = os.path.join(current_dir, "script2.py")
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Запуск 2.py...")

    try:
        result = subprocess.run(
            ["python3", script_path],
            capture_output=True,
            text=True,
            cwd=current_dir,
            timeout=300
        )

        if result.stdout:
            print(f"Вывод:\n{result.stdout}")
        if result.stderr:
            error_lines = result.stderr.strip().split('\n')
            last_error = error_lines[-1] if error_lines else result.stderr
            print(f"Ошибка: {last_error}")

        if result.returncode == 0:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✓ Успешно завершено\n")
            return True
        else:
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Завершено с ошибкой (код: {result.returncode})\n")
            return False

    except subprocess.TimeoutExpired:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Превышен таймаут\n")
        return False
    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] ✗ Ошибка: {e}\n")
        return False

def run_with_retry():
    if not RETRY_ON_ERROR:
        run_script()
        return

    for attempt in range(1, MAX_RETRIES + 1):
        success = run_script()
        if success:
            break
        if attempt < MAX_RETRIES:
            print(f"Попытка {attempt}/{MAX_RETRIES} не удалась. Повтор через {RETRY_DELAY_MINUTES} мин...\n")
            time.sleep(RETRY_DELAY_MINUTES * 60)
        else:
            print(f"Все {MAX_RETRIES} попытки исчерпаны.\n")

print("=" * 60)
print("Планировщик запуска 2.py")
print("=" * 60)
print(f"Интервал запуска: каждые {INTERVAL_MINUTES} минут")
print(f"Повтор при ошибке: {'Да' if RETRY_ON_ERROR else 'Нет'}")
print("Нажмите Ctrl+C для остановки.")
print("=" * 60 + "\n")

try:
    while True:
        run_with_retry()

        next_run = datetime.now().timestamp() + (INTERVAL_MINUTES * 60)
        next_run_str = datetime.fromtimestamp(next_run).strftime('%Y-%m-%d %H:%M:%S')

        print(f"Следующий запуск: {next_run_str}")
        print(f"Ожидание {INTERVAL_MINUTES} минут...\n")

        time.sleep(INTERVAL_MINUTES * 60)

except KeyboardInterrupt:
    print("\nПланировщик остановлен.")
