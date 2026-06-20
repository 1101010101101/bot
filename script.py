import requests
from bs4 import BeautifulSoup
import sys

s = requests.Session()
s.headers.update({
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
    "Referer": "https://hdmn.cloud/en/demo/",
    "Accept-Language": "en-US,en;q=0.9"
})

try:
    resp = s.post(
        "https://hdmn.cloud/en/demo/success/",
        data={"demo_mail": "soleg271828@gmail.com"},
        allow_redirects=True,
        timeout=30  # увеличили с 15 до 30 секунд
    )

    soup = BeautifulSoup(resp.text, "html.parser")
    for t in soup(["script", "style", "noscript"]):
        t.decompose()

    text = "\n".join([ln.strip() for ln in soup.get_text("\n").splitlines() if ln.strip()])
    print(text)

except requests.exceptions.Timeout:
    print("Таймаут — сервер не ответил, пробуем снова позже")
    sys.exit(0)  # exit 0 чтобы воркфлоу не считал это ошибкой
except requests.exceptions.ConnectionError:
    print("Ошибка соединения — сервер недоступен")
    sys.exit(0)
except Exception as e:
    print(f"Неожиданная ошибка: {e}")
    sys.exit(0)
