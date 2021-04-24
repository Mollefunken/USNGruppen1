
CREATE TABLE Bruker (
	epostadresse text NOT NULL,
	passordhash text NOT NULL,
	fornavn text NOT NULL,
	etternavn text NOT NULL,
	PRIMARY KEY(epostadresse)
);

DROP TABLE IF EXISTS Sesjon;

CREATE TABLE Sesjon (
	sesjonsID text NOT NULL,
	epostadresse text NOT NULL,
	PRIMARY KEY(sesjonsID),
	FOREIGN KEY(epostadresse)
		REFERENCES Bruker(epostadresse)
		ON UPDATE CASCADE
		ON DELETE CASCADE,
	UNIQUE(sesjonsID)
);


CREATE TABLE Dikt (
	diktID integer NOT NULL,
	dikt text NOT NULL,
	epostadresse text NOT NULL,
	PRIMARY KEY(diktID),
	FOREIGN KEY(epostadresse)
		REFERENCES Bruker(epostadresse)
		ON UPDATE CASCADE
		ON DELETE CASCADE,
	UNIQUE(diktID)
);

PRAGMA foreign_keys = ON;
