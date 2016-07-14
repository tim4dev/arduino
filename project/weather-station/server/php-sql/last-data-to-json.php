<?php
/*
 * Извлекаем последние записи из БД и отдаем в формате JSON
 *
 * Проект http://tim4dev.com/tag/meteostantsiya/
 * 
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

/*
 * Вызов скрипта:
 * .../last-data-to-json.php?k=<access_key>
 *
 
 Пример ответа в формате JSON :
 
 {
    "DHT 11":{
        "idSensor":"11",
        "dateCreate":"2016-04-20 18:06:03",
        "temperature":"19",
        "humidity":"26",
        "voltage":"5.01"
    },
    "DHT 20":{
        "idSensor":"20",
        "dateCreate":"2016-04-18 07:36:26",
        "temperature":"10",
        "humidity":"26",
        "voltage":"3.7"
    },
    "BMP 11":{
        "idSensor":"11",
        "dateCreate":"2016-04-20 18:06:22",
        "temperature":"19",
        "pressure":"987.97"
    }
}
 
 */
 
define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );
require_once( ABSPATH . 'function.php' );

// проверка правомочности доступа
checkAccess();

$db = mysqli_connect($dbhost, $dbuser, $dbpassword, $dbname);
/* check connection */
if ( mysqli_connect_errno() ) {
    echo 'ERROR. mysqli_connect_error ', mysqli_connect_error();
    exit();
}

$data_all = array();

// в зависимости от типа и id датчика, извлекаем данные из соответствующей таблицы

$idSensor = 11; // домашний DHT датчик
$query = "SELECT idSensor, dateCreate, temperature, humidity, voltage FROM arduino_dht WHERE idSensor = $idSensor ORDER BY id DESC LIMIT 1";
$res = mysqli_query($db, $query);
while ($r = mysqli_fetch_assoc($res) ) {
    $data_all["DHT $idSensor"] = $r;
}

$idSensor = 20; // заоконный DHT датчик
$query = "SELECT idSensor, dateCreate, temperature, humidity, voltage FROM arduino_dht WHERE idSensor = $idSensor ORDER BY id DESC LIMIT 1";
$res = mysqli_query($db, $query);
while ($r = mysqli_fetch_assoc($res) ) {
    $data_all["DHT $idSensor"] = $r;
}

$idSensor = 11; // домашний BMP датчик
$query = "SELECT idSensor, dateCreate, temperature, pressure FROM arduino_bmp WHERE idSensor = $idSensor ORDER BY id DESC LIMIT 1";
$res = mysqli_query($db, $query);
while ($r = mysqli_fetch_assoc($res) ) {
    $data_all["BMP $idSensor"] = $r;
}

// документация http://php.net/manual/ru/function.json-encode.php
echo json_encode($data_all);

mysqli_close($db);

?>
