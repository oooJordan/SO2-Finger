# 👤 SO2-Finger

Replica in linguaggio **C** del comando Unix `finger`, sviluppata per il corso di **Sistemi Operativi II** – Università La Sapienza.

Il comando `finger` permette di visualizzare informazioni sugli utenti registrati in un sistema Unix/Linux.

---

## Obiettivo del progetto

Simulare il comportamento base del comando `finger`:

- Mostrare informazioni sugli utenti locali attivi
- Stampare dettagli come:
  - Nome utente
  - Shell
  - Directory home
  - Ultimo login
  - Contenuto del file `~/.plan` se presente


L'implementazione accede ai file standard del sistema: `/etc/passwd`, `/var/run/utmp`, e alle home degli utenti per leggere `~/.plan`.

---

## Compilazione

Assicurati di avere `gcc` installato.

Per compilare:

```bash
gcc -Wall -o finger main.c
```

## Esecuzione
```bash
./finger [opzioni] [utente]
./finger [opzioni]
./finger
```

