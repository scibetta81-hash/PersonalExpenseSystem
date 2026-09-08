#include <sqlite3.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>
#include <limits>

using namespace std;

sqlite3* db = nullptr;

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

bool soloCifre(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool formatoDataValido(const string& data) {
    if (data.size() != 10 || data[4] != '-' || data[7] != '-') return false;
    string anno = data.substr(0, 4);
    string mese = data.substr(5, 2);
    string giorno = data.substr(8, 2);
    if (!soloCifre(anno) || !soloCifre(mese) || !soloCifre(giorno)) return false;
    int m = stoi(mese);
    int g = stoi(giorno);
    return m >= 1 && m <= 12 && g >= 1 && g <= 31;
}

bool formatoMeseValido(const string& mese) {
    if (mese.size() != 7 || mese[4] != '-') return false;
    string anno = mese.substr(0, 4);
    string mm = mese.substr(5, 2);
    if (!soloCifre(anno) || !soloCifre(mm)) return false;
    int m = stoi(mm);
    return m >= 1 && m <= 12;
}

bool leggiImportoPositivo(const string& messaggio, double& valore) {
    cout << messaggio;
    string input;
    getline(cin, input);
    stringstream ss(input);
    ss >> valore;
    return ss && ss.eof() && valore > 0;
}

bool eseguiSQL(const string& sql) {
    char* errore = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errore);
    if (rc != SQLITE_OK) {
        cerr << "Errore SQL: " << (errore ? errore : "errore sconosciuto") << endl;
        sqlite3_free(errore);
        return false;
    }
    return true;
}

bool inizializzaDatabase() {
    if (sqlite3_open("personal_expenses.db", &db) != SQLITE_OK) {
        cerr << "Errore nell'apertura del database." << endl;
        return false;
    }

    if (!eseguiSQL("PRAGMA foreign_keys = ON;")) return false;

    const string schema = R"SQL(
        CREATE TABLE IF NOT EXISTS Categoria (
            IDCategoria INTEGER PRIMARY KEY,
            Nome VARCHAR(50) NOT NULL UNIQUE
        );

        CREATE TABLE IF NOT EXISTS Spesa (
            IDSpesa INTEGER PRIMARY KEY,
            Data CHAR(10) NOT NULL,
            Importo DECIMAL(10,2) NOT NULL CHECK (Importo > 0),
            IDCategoria INTEGER NOT NULL,
            Descrizione VARCHAR(255),
            FOREIGN KEY (IDCategoria) REFERENCES Categoria(IDCategoria)
        );

        CREATE TABLE IF NOT EXISTS Budget (
            Mese CHAR(7) NOT NULL,
            IDCategoria INTEGER NOT NULL,
            ImportoBudget DECIMAL(10,2) NOT NULL CHECK (ImportoBudget > 0),
            PRIMARY KEY (Mese, IDCategoria),
            FOREIGN KEY (IDCategoria) REFERENCES Categoria(IDCategoria)
        );
    )SQL";

    return eseguiSQL(schema);
}

bool trovaCategoria(const string& nome, int& idCategoria) {
    const char* sql = "SELECT IDCategoria FROM Categoria WHERE Nome = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);

    bool trovata = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        idCategoria = sqlite3_column_int(stmt, 0);
        trovata = true;
    }
    sqlite3_finalize(stmt);
    return trovata;
}

void gestioneCategorie() {
    cout << "\n--- GESTIONE CATEGORIE ---\n";
    cout << "Nome della categoria: ";
    string nome;
    getline(cin, nome);
    nome = trim(nome);

    if (nome.empty()) {
        cout << "Errore: il nome della categoria non puo' essere vuoto.\n";
        return;
    }

    int id = 0;
    if (trovaCategoria(nome, id)) {
        cout << "La categoria esiste gia'.\n";
        return;
    }

    const char* sql = "INSERT INTO Categoria (Nome) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Errore durante la preparazione dell'inserimento.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_DONE)
        cout << "Categoria inserita correttamente.\n";
    else
        cout << "Errore durante l'inserimento della categoria.\n";

    sqlite3_finalize(stmt);
}

void inserisciSpesa() {
    cout << "\n--- INSERIMENTO SPESA ---\n";

    string data;
    cout << "Data (YYYY-MM-DD): ";
    getline(cin, data);
    data = trim(data);
    if (!formatoDataValido(data)) {
        cout << "Errore: formato data non valido.\n";
        return;
    }

    double importo = 0.0;
    if (!leggiImportoPositivo("Importo: ", importo)) {
        cout << "Errore: l'importo deve essere maggiore di zero.\n";
        return;
    }

    string nomeCategoria;
    cout << "Nome della categoria: ";
    getline(cin, nomeCategoria);
    nomeCategoria = trim(nomeCategoria);

    int idCategoria = 0;
    if (!trovaCategoria(nomeCategoria, idCategoria)) {
        cout << "Errore: la categoria non esiste.\n";
        return;
    }

    string descrizione;
    cout << "Descrizione facoltativa: ";
    getline(cin, descrizione);
    descrizione = trim(descrizione);

    const char* sql =
        "INSERT INTO Spesa (Data, Importo, IDCategoria, Descrizione) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Errore durante la preparazione dell'inserimento.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, data.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, importo);
    sqlite3_bind_int(stmt, 3, idCategoria);
    if (descrizione.empty())
        sqlite3_bind_null(stmt, 4);
    else
        sqlite3_bind_text(stmt, 4, descrizione.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        cout << "Spesa inserita correttamente.\n";
    else
        cout << "Errore durante l'inserimento della spesa.\n";

    sqlite3_finalize(stmt);
}

bool budgetEsistente(const string& mese, int idCategoria) {
    const char* sql = "SELECT 1 FROM Budget WHERE Mese = ? AND IDCategoria = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, mese.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, idCategoria);
    bool esiste = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return esiste;
}

void definisciBudget() {
    cout << "\n--- DEFINIZIONE BUDGET MENSILE ---\n";

    string mese;
    cout << "Mese (YYYY-MM): ";
    getline(cin, mese);
    mese = trim(mese);
    if (!formatoMeseValido(mese)) {
        cout << "Errore: formato mese non valido.\n";
        return;
    }

    string nomeCategoria;
    cout << "Nome della categoria: ";
    getline(cin, nomeCategoria);
    nomeCategoria = trim(nomeCategoria);

    int idCategoria = 0;
    if (!trovaCategoria(nomeCategoria, idCategoria)) {
        cout << "Errore: la categoria non esiste.\n";
        return;
    }

    double importoBudget = 0.0;
    if (!leggiImportoPositivo("Importo del budget: ", importoBudget)) {
        cout << "Errore: il budget deve essere maggiore di zero.\n";
        return;
    }

    sqlite3_stmt* stmt = nullptr;
    if (budgetEsistente(mese, idCategoria)) {
        const char* sql =
            "UPDATE Budget SET ImportoBudget = ? WHERE Mese = ? AND IDCategoria = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cout << "Errore durante la preparazione dell'aggiornamento.\n";
            return;
        }
        sqlite3_bind_double(stmt, 1, importoBudget);
        sqlite3_bind_text(stmt, 2, mese.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, idCategoria);
    } else {
        const char* sql =
            "INSERT INTO Budget (Mese, IDCategoria, ImportoBudget) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cout << "Errore durante la preparazione dell'inserimento.\n";
            return;
        }
        sqlite3_bind_text(stmt, 1, mese.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, idCategoria);
        sqlite3_bind_double(stmt, 3, importoBudget);
    }

    if (sqlite3_step(stmt) == SQLITE_DONE)
        cout << "Budget mensile salvato correttamente.\n";
    else
        cout << "Errore durante il salvataggio del budget.\n";

    sqlite3_finalize(stmt);
}

void reportTotalePerCategoria() {
    cout << "\n--- TOTALE SPESE PER CATEGORIA ---\n";
    cout << left << setw(25) << "Categoria" << right << setw(15) << "Totale Speso" << '\n';
    cout << string(40, '-') << '\n';

    const char* sql = R"SQL(
        SELECT C.Nome, SUM(S.Importo)
        FROM Categoria C, Spesa S
        WHERE C.IDCategoria = S.IDCategoria
        GROUP BY C.IDCategoria, C.Nome
        ORDER BY C.Nome;
    )SQL";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Errore nell'esecuzione del report.\n";
        return;
    }

    bool almenoUna = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        almenoUna = true;
        string categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double totale = sqlite3_column_double(stmt, 1);
        cout << left << setw(25) << categoria << right << setw(15)
             << fixed << setprecision(2) << totale << '\n';
    }
    if (!almenoUna) cout << "Nessuna spesa registrata.\n";
    sqlite3_finalize(stmt);
}

double totaleSpesoMeseCategoria(const string& mese, int idCategoria) {
    const char* sql =
        "SELECT SUM(Importo) FROM Spesa WHERE IDCategoria = ? AND Data LIKE ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0.0;

    string pattern = mese + "%";
    sqlite3_bind_int(stmt, 1, idCategoria);
    sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);

    double totale = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
        totale = sqlite3_column_double(stmt, 0);

    sqlite3_finalize(stmt);
    return totale;
}

void reportSpeseVsBudget() {
    cout << "\n--- SPESE MENSILI VS BUDGET ---\n";
    string mese;
    cout << "Mese da analizzare (YYYY-MM): ";
    getline(cin, mese);
    mese = trim(mese);
    if (!formatoMeseValido(mese)) {
        cout << "Errore: formato mese non valido.\n";
        return;
    }

    const char* sql = R"SQL(
        SELECT B.IDCategoria, C.Nome, B.ImportoBudget
        FROM Budget B, Categoria C
        WHERE B.IDCategoria = C.IDCategoria AND B.Mese = ?
        ORDER BY C.Nome;
    )SQL";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Errore nell'esecuzione del report.\n";
        return;
    }
    sqlite3_bind_text(stmt, 1, mese.c_str(), -1, SQLITE_TRANSIENT);

    bool almenoUno = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        almenoUno = true;
        int idCategoria = sqlite3_column_int(stmt, 0);
        string categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double budget = sqlite3_column_double(stmt, 2);
        double speso = totaleSpesoMeseCategoria(mese, idCategoria);

        cout << "\nMese: " << mese << '\n';
        cout << "Categoria: " << categoria << '\n';
        cout << "Budget: " << fixed << setprecision(2) << budget << '\n';
        cout << "Speso: " << fixed << setprecision(2) << speso << '\n';
        if (speso > budget)
            cout << "Stato: SUPERAMENTO BUDGET\n";
        else
            cout << "Stato: BUDGET RISPETTATO\n";
    }

    if (!almenoUno) cout << "Nessun budget definito per il mese indicato.\n";
    sqlite3_finalize(stmt);
}

void reportElencoSpese() {
    cout << "\n--- ELENCO COMPLETO DELLE SPESE ---\n";
    cout << left << setw(13) << "Data"
         << setw(22) << "Categoria"
         << right << setw(12) << "Importo" << "  "
         << left << "Descrizione" << '\n';
    cout << string(75, '-') << '\n';

    const char* sql = R"SQL(
        SELECT S.Data, C.Nome, S.Importo, S.Descrizione
        FROM Spesa S, Categoria C
        WHERE S.IDCategoria = C.IDCategoria
        ORDER BY S.Data;
    )SQL";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "Errore nell'esecuzione del report.\n";
        return;
    }

    bool almenoUna = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        almenoUna = true;
        string data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string categoria = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double importo = sqlite3_column_double(stmt, 2);
        const unsigned char* descrizioneTxt = sqlite3_column_text(stmt, 3);
        string descrizione = descrizioneTxt ? reinterpret_cast<const char*>(descrizioneTxt) : "";

        cout << left << setw(13) << data
             << setw(22) << categoria
             << right << setw(12) << fixed << setprecision(2) << importo << "  "
             << left << descrizione << '\n';
    }
    if (!almenoUna) cout << "Nessuna spesa registrata.\n";
    sqlite3_finalize(stmt);
}

void menuReport() {
    int scelta = 0;
    do {
        cout << "\n-------------------------\n";
        cout << "MENU REPORT\n";
        cout << "-------------------------\n";
        cout << "1. Totale spese per categoria\n";
        cout << "2. Spese mensili vs budget\n";
        cout << "3. Elenco completo delle spese ordinate per data\n";
        cout << "4. Ritorna al menu principale\n";
        cout << "-------------------------\n";
        cout << "Inserisci la tua scelta: ";

        if (!(cin >> scelta)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Scelta non valida. Riprovare.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (scelta) {
            case 1: reportTotalePerCategoria(); break;
            case 2: reportSpeseVsBudget(); break;
            case 3: reportElencoSpese(); break;
            case 4: break;
            default: cout << "Scelta non valida. Riprovare.\n";
        }
    } while (scelta != 4);
}

int main() {
    if (!inizializzaDatabase()) {
        if (db) sqlite3_close(db);
        return 1;
    }

    cout << "Benvenuto nel Sistema di Gestione delle Spese Personali e del Budget.\n";

    int scelta = 0;
    do {
        cout << "\n-------------------------\n";
        cout << "SISTEMA SPESE PERSONALI\n";
        cout << "-------------------------\n";
        cout << "1. Gestione Categorie\n";
        cout << "2. Inserisci Spesa\n";
        cout << "3. Definisci Budget Mensile\n";
        cout << "4. Visualizza Report\n";
        cout << "5. Esci\n";
        cout << "-------------------------\n";
        cout << "Inserisci la tua scelta: ";

        if (!(cin >> scelta)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Scelta non valida. Riprovare.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (scelta) {
            case 1: gestioneCategorie(); break;
            case 2: inserisciSpesa(); break;
            case 3: definisciBudget(); break;
            case 4: menuReport(); break;
            case 5: cout << "Chiusura del programma.\n"; break;
            default: cout << "Scelta non valida. Riprovare.\n";
        }
    } while (scelta != 5);

    sqlite3_close(db);
    return 0;
}
