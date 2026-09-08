PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS Budget;
DROP TABLE IF EXISTS Spesa;
DROP TABLE IF EXISTS Categoria;

CREATE TABLE Categoria (
    IDCategoria INTEGER PRIMARY KEY,
    Nome VARCHAR(50) NOT NULL UNIQUE
);

CREATE TABLE Spesa (
    IDSpesa INTEGER PRIMARY KEY,
    Data CHAR(10) NOT NULL,
    Importo DECIMAL(10,2) NOT NULL CHECK (Importo > 0),
    IDCategoria INTEGER NOT NULL,
    Descrizione VARCHAR(255),
    FOREIGN KEY (IDCategoria) REFERENCES Categoria(IDCategoria)
);

CREATE TABLE Budget (
    Mese CHAR(7) NOT NULL,
    IDCategoria INTEGER NOT NULL,
    ImportoBudget DECIMAL(10,2) NOT NULL CHECK (ImportoBudget > 0),
    PRIMARY KEY (Mese, IDCategoria),
    FOREIGN KEY (IDCategoria) REFERENCES Categoria(IDCategoria)
);

INSERT INTO Categoria (IDCategoria, Nome) VALUES (1, 'Alimentari');
INSERT INTO Categoria (IDCategoria, Nome) VALUES (2, 'Trasporti');
INSERT INTO Categoria (IDCategoria, Nome) VALUES (3, 'Casa');
INSERT INTO Categoria (IDCategoria, Nome) VALUES (4, 'Svago');

INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (1, '2025-01-05', 45.50, 1, 'Spesa supermercato');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (2, '2025-01-10', 60.00, 2, 'Carburante');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (3, '2025-01-15', 25.00, 1, 'Pranzo');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (4, '2025-01-20', 280.00, 1, 'Spesa mensile');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (5, '2025-01-22', 60.00, 2, 'Abbonamento trasporti');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (6, '2025-02-03', 55.00, 3, 'Utenza domestica');
INSERT INTO Spesa (IDSpesa, Data, Importo, IDCategoria, Descrizione)
VALUES (7, '2025-02-14', 35.00, 4, 'Cinema e cena');

INSERT INTO Budget (Mese, IDCategoria, ImportoBudget) VALUES ('2025-01', 1, 300.00);
INSERT INTO Budget (Mese, IDCategoria, ImportoBudget) VALUES ('2025-01', 2, 150.00);
INSERT INTO Budget (Mese, IDCategoria, ImportoBudget) VALUES ('2025-02', 3, 200.00);
INSERT INTO Budget (Mese, IDCategoria, ImportoBudget) VALUES ('2025-02', 4, 100.00);
