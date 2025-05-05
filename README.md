# ArduinoHodiny
Jak spustit projekt?
Tento projekt propojuje Google Kalendář s Arduinem a převádí události na „čas v minutách“. Níže je kompletní návod k nasazení.

Instalace Pythonu
Nejprve si stáhněte a nainstalujte Python verze 3.12 z oficiální stránky:
  https://www.python.org/downloads/release/python-3120/
Při instalaci zaškrtněte volbu "Add Python to PATH", jinak nebude fungovat v příkazové řádce.

Nainstalujte potřebné knihovny
Otevřete terminál / příkazový řádek a nainstalujte následující knihovny:
bash
ZkopírovatUpravit
pip install google-auth google-auth-oauthlib google-api-python-client pyserial


Nastavení Google API
Zprovozněte Google Calendar API podle oficiální dokumentace:
 👉 Google Calendar API quickstart - Python
Vytvořte projekt v Google Cloud Console


Aktivujte Google Calendar API


Vytvořte OAuth 2.0 Client ID


Stáhněte si soubor credentials.json


Přidejte credentials.json do projektu
Ve složce s projektem najdete adresář credentials. Nahraďte jeho obsah souborem credentials.json, který jste stáhli z Google API.
Nezapomeňte:
Přidat svůj Google účet do testovacích uživatelů v Google Cloud Console (jinak ověření nebude fungovat)



Spusťte Python kód
Spusťte skript pomocí:
bash
ZkopírovatUpravit
python nazev_souboru.py

Po prvním spuštění:
se otevře prohlížeč s výzvou k přihlášení a ověření přístupu,


po úspěšném ověření se vytvoří soubor token.json, který uchovává přístupový token.



Vytvořte události v Google Kalendáři
Zadejte do svého kalendáře události s konkrétními časy. Po spuštění skriptu budou tyto události převedeny na minuty a odeslány do Arduina.

Instalace Arduino IDE a knihoven
Stáhněte si Arduino IDE z oficiální stránky:
  https://www.arduino.cc/en/software


Otevřete adresář s Arduino kódem (soubor .ino).
Přetáhněte potřebné knihovny do složky libraries, která se nachází ve složce Arduino, tyto knihovny najdete přiložené v projektu.




Zapojení a spuštění
Zapojte hardware podle schématu, které bylo uvedeno v sekci Realizace (nezapomeňte vložit baterii do RTC modulu).


Nahrajte kód do Arduina pomocí Arduino IDE. (nezapomeňte vybrat arduino uno)


Ujistěte se, že máte události v Google Kalendáři a že Arduino je připojené (např. na portu COM3).


Spusťte opět Python skript – data se převedou a odešlou do zařízení.
