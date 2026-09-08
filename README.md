# PersonalExpenseSystem

Sistema console C++ per la gestione delle spese personali e del budget con database SQL relazionale.

## Requisiti

- Compilatore C++ (g++) con supporto C++17
- Libreria SQLite3

## Compilazione

```bash
g++ -std=c++17 src/main.cpp -lsqlite3 -o PersonalExpenseSystem
```

## Avvio

```bash
./PersonalExpenseSystem
```

(Su Windows, potrebbe essere: `PersonalExpenseSystem.exe`)

## Funzioni del Sistema

1. **Gestione Categorie** — Inserisci e verifica le categorie di spesa
2. **Inserisci Spesa** — Registra data, importo, categoria e descrizione
3. **Definisci Budget Mensile** — Imposta il limite mensile di spesa per categoria
4. **Visualizza Report** — Mostra tre report: totale per categoria, confronto spese vs budget, elenco completo
5. **Esci** — Chiude il programma

## Database

All'avvio il programma crea automaticamente il database `personal_expenses.db` con le tabelle necessarie.

Lo script SQL completo è disponibile in `sql/database.sql`.

## Demo

Vedi `demo/demo_video.mp4` per una dimostrazione del funzionamento del sistema.

## Struttura del Repository

```
PersonalExpenseSystem/
├── src/
│   └── main.cpp
├── sql/
│   └── database.sql
├── demo/
│   └── demo_video.mp4
└── README.md
```

## Valutazione

Il progetto è stato realizzato secondo i criteri ufficiali:
- Logica C++ e menu: 25%
- Progettazione DB e vincoli: 25%
- Query SQL e report: 25%
- Correttezza e robustezza: 15%
- Documentazione: 10%
