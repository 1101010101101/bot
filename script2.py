import requests
from bs4 import BeautifulSoup
from datetime import datetime

EMAIL = "bot.hat.tg.oleg@gmail.com"
URL = "https://hdmn.cloud/en/demo/success/"
HEADERS = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}

def log(msg):
    print(f"[{datetime.now().strftime('%H:%M:%S')}] {msg}")

def run():
    s = requests.Session()
    s.headers.update(HEADERS)

    log("Отправляем запрос...")
    resp = s.post(URL, data={"demo_mail": EMAIL}, allow_redirects=True, timeout=15)

    log(f"Status: {resp.status_code}")
    log(f"URL: {resp.url}")

    if resp.history:
        log(f"Редиректы: {' → '.join(r.url for r in resp.history)}")

    log(f"Размер: {len(resp.text)} символов")
    print("-" * 50)

    soup = BeautifulSoup(resp.text, "html.parser")
    for t in soup(["script", "style", "noscript"]):
        t.decompose()

    text = "\n".join(ln.strip() for ln in soup.get_text("\n").splitlines() if ln.strip())
    print(text)
    print("-" * 50)

if __name__ == "__main__":
    run()
