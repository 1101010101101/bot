import requests
import time
from bs4 import BeautifulSoup
from datetime import datetime

EMAIL = "bot.hat.tg.oleg@gmail.com"
HEADERS = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
BASE = "https://hdmn.cloud"
RAILWAY_TOKEN = "bc789e58-d32a-4449-a5f6-41901f51c919"
SERVICE_ID = "c4f351b6-882b-4742-bdee-b4a5859a6fef"
ENVIRONMENT_ID = "f3664da9-967c-47b3-8c30-69a77374e575"

DELAY_MINUTES = 0
MAX_ATTEMPTS = 5


def log(msg):
    print(f"[{datetime.now().strftime('%H:%M:%S')}] {msg}")


def redeploy():
    log(f"Ждём {DELAY_MINUTES} минут перед перезапуском...")
    time.sleep(DELAY_MINUTES * 60)

    for attempt in range(1, MAX_ATTEMPTS + 1):
        log(f"🔄 Попытка {attempt}/{MAX_ATTEMPTS} — запускаем деплой...")

        try:
            r = requests.post(
                "https://backboard.railway.app/graphql/v2",
                headers={
                    "Authorization": f"Bearer {RAILWAY_TOKEN}",
                    "Content-Type": "application/json"
                },
                json={
                    "query": """
                        mutation {
                            serviceInstanceRedeploy(
                                serviceId: "%s"
                                environmentId: "%s"
                            )
                        }
                    """ % (SERVICE_ID, ENVIRONMENT_ID)
                },
                timeout=30
            )

            if r.status_code == 200:
                log(f"✅ Redeploy успешно запущен: {r.text}")
                return

            elif r.status_code == 429:
                wait = 2 ** attempt * 5  # 10, 20, 40, 80, 160 минут
                log(f"⚠️ Rate limited (429). Ждём {wait} минут...")
                time.sleep(wait * 60)

            else:
                log(f"❌ Неожиданный статус {r.status_code}: {r.text}")
                return

        except requests.RequestException as e:
            log(f"❌ Ошибка запроса: {e}")
            return

    log("🚫 Все попытки исчерпаны. Redeploy не выполнен.")


def run():
    s = requests.Session()
    s.headers.update(HEADERS)

    log("Проверяем текущий статус...")
    page = s.get(f"{BASE}/en/demo/success/", timeout=15)
    soup = BeautifulSoup(page.text, "html.parser")

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
        log("Ссылка отмены не найдена")

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
    redeploy()
