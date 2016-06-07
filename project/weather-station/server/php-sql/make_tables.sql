

USE db_weather;

CREATE TABLE IF NOT EXISTS arduino_dht (
	id		    INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
	ipRemote	INTEGER UNSIGNED,
	idSensor	INTEGER,
	dateCreate	DATETIME NOT NULL,
	millis      INTEGER UNSIGNED,
	temperature	DOUBLE,
	humidity	DOUBLE,
	voltage		DOUBLE,
	errors		INTEGER,

	PRIMARY KEY(id),
	INDEX idxDateCreate1(dateCreate)
);

CREATE TABLE IF NOT EXISTS arduino_bmp (
    id          INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
    ipRemote    INTEGER UNSIGNED,
    idSensor    INTEGER,
    dateCreate  DATETIME NOT NULL,
    millis      INTEGER UNSIGNED,
    temperature DOUBLE,
    pressure    DOUBLE,
    errors      INTEGER,

    PRIMARY KEY(id),
    INDEX idxDateCreate2(dateCreate)
);

CREATE TABLE IF NOT EXISTS arduino_error_log (
    id          INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
    dateCreate  DATETIME NOT NULL,
    ipRemote    INTEGER UNSIGNED,
    msg         VARCHAR(1024),

    PRIMARY KEY(id),
    INDEX idxDateCreate3(dateCreate)
);
