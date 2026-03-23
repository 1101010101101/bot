import requests
import time
from bs4 import BeautifulSoup

def run():
    s = requests.Session()
    s.headers.update({"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"})

    print("[DEBUG] Отправляем запрос...")

    resp = s.post(
        "https://hdmn.cloud/en/demo/success/",
        data={"demo_mail": "bot.hat.tg.oleg@gmail.com"},
        allow_redirects=True,
        timeout=15
    )

    print(f"[DEBUG] Status code: {resp.status_code}")
    print(f"[DEBUG] Финальный URL: {resp.url}")
    print(f"[DEBUG] Редиректы: {[r.url for r in resp.history]}")
    print(f"[DEBUG] Размер ответа: {len(resp.text)} символов")
    print(f"[DEBUG] Первые 500 символов HTML:\n{resp.text[:500]}")
    print("-" * 50)

    soup = BeautifulSoup(resp.text, "html.parser")
    for t in soup(["script", "style", "noscript"]):
        t.decompose()

    text = "\n".join([ln.strip() for ln in soup.get_text("\n").splitlines() if ln.strip()])
    print(text)

while True:
    try:
        run()
    except Exception as e:
        print(f"Ошибка: {e}")
    print("Ждём 5 минут...")
    time.sleep(300)
