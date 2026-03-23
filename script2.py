import requests
from bs4 import BeautifulSoup
from datetime import datetime

EMAIL = "bot.hat.tg.oleg@gmail.com"
HEADERS = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
BASE = "https://hdmn.cloud"

def log(msg):
    print(f"[{datetime.now().strftime('%H:%M:%S')}] {msg}")

def run():
    # Новая сессия каждый раз — чистые куки
    s = requests.Session()
    s.headers.update(HEADERS)

    # Шаг 1: заходим на страницу success чтобы найти ссылку отмены
    log("Проверяем текущий статус...")
    page = s.get(f"{BASE}/en/demo/success/", timeout=15)
    soup = BeautifulSoup(page.text, "html.parser")

    # Ищем ссылку cancel на странице
    cancel_link = None
    for a in soup.find_all("a", href=True):
        if "cancel" in a["href"].lower():
            cancel_link = a["href"]
            break

    if cancel_link:
        if not cancel_link.startswith("http"):
            cancel_link = BASE + cancel_link
        log(f"Отменяем: {cancel_link}")
        s.get(cancel_link, timeout=15)
    else:
        log("Ссылка отмены не найдена — пробуем напрямую")

    # Шаг 2: запрашиваем новый код
    log("Запрашиваем новый код...")
    resp = s.post(
        f"{BASE}/en/demo/success/",
        data={"demo_mail": EMAIL},
        allow_redirects=True,
        timeout=15
    )

    log(f"Status: {resp.status_code} | URL: {resp.url}")
    print("-" * 50)

    soup2 = BeautifulSoup(resp.text, "html.parser")
    for t in soup2(["script", "style", "noscript"]):
        t.decompose()

    text = "\n".join(ln.strip() for ln in soup2.get_text("\n").splitlines() if ln.strip())
    print(text)
    print("-" * 50)

if __name__ == "__main__":
    run()
