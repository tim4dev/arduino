<?php
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

$dbhost      = 'localhost';
$dbuser      = 'u_weather';
$dbpassword  = '***PASSWORD***';  // см. make_db.sql
$dbname      = 'db_weather';
$dbtable_dht = 'arduino_dht';  // таблица данных DHT датчиков
$dbtable_bmp = 'arduino_bmp';  // таблица данных BMP датчика

$access_key  = '***KEY***'; // секретный ключ -- число, должен совпадать с константой SOURCE_KEY из скетча server.ino

$SQL_LIMIT   = 10000;  // лимит вывода строк на веб-странице

$dbtable_error_log = 'arduino_error_log'; // таблица для логов
$ERROR_LOG   = 1;   // вести ли лог в таблице arduino_error_log

date_default_timezone_set('Europe/Minsk');

?>
