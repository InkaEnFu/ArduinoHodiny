import datetime
import os.path
import serial
import time

# Knihovny pro autentizaci a práci s Google Calendar API
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from googleapiclient.errors import HttpError

# Oprávnění k načítání z Google Kalendáře (pouze čtení)
SCOPES = ["https://www.googleapis.com/auth/calendar.readonly"]

# Nastavení sériového portu pro komunikaci s Arduinem
arduino = serial.Serial(port='COM3', baudrate='9600', timeout=.1)

# Funkce pro odeslání času události do Arduina
def sendToArduino(event_time, event_summary):
    current_time = datetime.datetime.now()
    current_hour = current_time.hour
    current_minute = current_time.minute

    print("Aktuální systémový čas:", current_hour, current_minute)

    # Získání hodin a minut z formátu "YYYY-MM-DDTHH:MM:SS"
    hours = int(event_time.split("T")[1].split(":")[0])
    minutes = int(event_time.split("T")[1].split(":")[1])
    print("Čas události:", hours, minutes)

    # Převod času události na minuty od půlnoci
    event_minutes = int(hours * 60 + minutes)
    print(f"Čas události v minutách: {event_minutes}")

    # Převod aktuálního času na minuty od půlnoci
    current_time_in_minutes = int(current_hour * 60 + current_minute)
    print("Aktuální čas v minutách:", current_time_in_minutes)

    # Pauza a odeslání dat do Arduina přes sériový port
    time.sleep(1)
    arduino.write((f"{event_minutes},{current_time_in_minutes}\n").encode())

# Hlavní funkce pro načítání událostí z Google Kalendáře a jejich odeslání do Arduina
def main():
    creds = None

    # Načtení uložených přihlašovacích údajů (token.json)
    if os.path.exists("token.json"):
        creds = Credentials.from_authorized_user_file("token.json", SCOPES)

    # Pokud přihlašovací údaje nejsou platné, spusť novou autentizaci
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file("credentials.json", SCOPES)
            creds = flow.run_local_server(port=0)

        # Uložení nového tokenu
        with open("token.json", "w") as token:
            token.write(creds.to_json())

    try:
        # Vytvoření služby pro přístup ke Google Kalendáři
        service = build("calendar", "v3", credentials=creds)

        # Získání aktuálního času ve formátu ISO (UTC)
        now = datetime.datetime.now(tz=datetime.timezone.utc).isoformat()
        print("Získávání příštích 10 událostí")

        # Načtení 10 nadcházejících událostí z primárního kalendáře
        events_result = (
            service.events()
            .list(
                calendarId="primary",
                timeMin=now,
                maxResults=10,
                singleEvents=True,
                orderBy="startTime",
            )
            .execute()
        )

        events = events_result.get("items", [])

        if not events:
            print("Nenalezeny žádné nadcházející události.")
            return

        # Pro každou událost vypiš její začátek a název, a odešli ji do Arduina
        for event in events:
            start = event["start"].get("dateTime", event["start"].get("date"))
            print(start, event["summary"])
            sendToArduino(start, event["summary"])

    except HttpError as error:
        print(f"Došlo k chybě: {error}h")


# Spuštění programu
if __name__ == "__main__":
    main()
