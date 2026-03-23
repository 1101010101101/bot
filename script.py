import requests
from bs4 import BeautifulSoup

s = requests.Session()
s.headers.update({"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"})

resp = s.post(
    "https://hdmn.cloud/en/demo/success/",
    data={"demo_mail": "bot.hat.tg.oleg@gmail.com"},
    allow_redirects=True,
    timeout=15
)

soup = BeautifulSoup(resp.text, "html.parser")
for t in soup(["script", "style", "noscript"]):
    t.decompose()

text = "\n".join([ln.strip() for ln in soup.get_text("\n").splitlines() if ln.strip()])
print(text)
