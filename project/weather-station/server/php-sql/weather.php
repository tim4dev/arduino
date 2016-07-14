<?php
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );
require_once( ABSPATH . 'function.php' );

$db = mysqli_connect($dbhost, $dbuser, $dbpassword, $dbname);

/* check connection */
if ( mysqli_connect_errno() ) {
    echo "0\n", 'ERROR. mysqli_connect_error ', mysqli_connect_error();
    exit();
}

$ipRemote   = "INET_ATON('". $_SERVER['REMOTE_ADDR'] ."')";
$portRemote = $_SERVER['REMOTE_PORT'];
$dateCreate = date("Y-m-d H:i:s", time());

my_error_log( $_SERVER['REQUEST_URI'] );  // пишем лог

// проверка правомочности доступа
checkAccess();



/*************************************************************************
 * Функции
 **************************************************************************/



/* Структура данных типа dht
 * type = dht
 * s - id сенсора
 * t - температура
 * h - влажность
 * v - напряжение питания
 * m - millis() - количество миллисекунд с момента начала выполнения программы на плате Arduino
 * k - ключ
 * http://host/arduino/weather.php?type=dht&t=15&h=23&v=3.3&s=20&k=12345
 */
function save_dht()
{
    global $db, $dbtable_dht, $ipRemote, $portRemote, $dateCreate;
    $errors = 0;

    if ( isset($_GET["t"]) )    {
        $t = mysqli_real_escape_string($db, $_GET["t"]);
        if ( is_numeric($t) ) {
            $temperature = floatval($t);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["t"];
            $temperature = 0;
            $errors++;
        }
    } else {
        $temperature = 0;
        $errors++;
    }

    if ( isset($_GET["h"]) )    {
        $h = mysqli_real_escape_string($db, $_GET["h"]);
        if ( is_numeric($h) ) {
            $humidity = floatval($h);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["h"];
            $humidity = 0;
            $errors++;
        }
    } else {
        $humidity = 0;
        $errors++;
    }

    if ( isset($_GET["s"]) )    {
        $s = mysqli_real_escape_string($db, $_GET["s"]);
        if ( is_numeric($s) ) {
            $idSensor = floatval($s);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["s"];
            $idSensor = 0;
            $errors++;
        }
    } else {
        $idSensor = 0;
        $errors++;
    }

    if ( isset($_GET["v"]) )    {
        $v = mysqli_real_escape_string($db, $_GET["v"]);
        if ( is_numeric($v) ) {
            $voltage = floatval($v);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["v"];
            $voltage = 0;
            $errors++;
        }
    } else {
        $voltage = 0;
        $errors++;
    }

    if ( isset($_GET["m"]) )    {
        $m = mysqli_real_escape_string($db, $_GET["m"]);
        if ( is_numeric($m) ) {
            $millis = intval($m);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["m"];
            $millis = 0;
            $errors++;
        }
    } else {
        $millis = 0;
        $errors++;
    }

    if ( (null == $temperature) && (null == $humidity) && (null == $idSensor ) )  {
        echo "0\n", "WARNING. All value is NULL.";
        $errors++;
    }

    // запись данных таблицу DHT датчиков
    $query = "INSERT INTO $dbtable_dht (ipRemote, idSensor, dateCreate, temperature, humidity, voltage, millis, errors) ".
            ' VALUES ('. $ipRemote .", '". $idSensor ."', '". $dateCreate ."', ". $temperature .', '. $humidity .', '. $voltage .', '. $millis .', '. $errors .');';

    // debug
    //echo $query; echo '<br><br>'; exit;

    if ( !mysqli_query($db, $query) )   {
        echo "0\n", "ERROR. mysqli_query: ", mysqli_sqlstate($db), '<br />', mysqli_error($db);
        my_error_log( __FUNCTION__ .' ERROR. mysqli_sqlstate: ' . mysqli_sqlstate($db) .' mysqli_error: '. mysqli_error($db) );
        @ mysqli_close($db);
        exit();
    }   else {
        my_error_log( __FUNCTION__ ." OK" );
    }
}


/*
 * Структура данных типа bmp
 * type = bmp
 * s - id сенсора
 * t - температура
 * p - давление
 * m - millis() - количество миллисекунд с момента начала выполнения программы на плате Arduino
 * k - ключ
 * http://host/arduino/weather.php?type=bmp&t=15&p=1013&s=10&k=12345
 */
function save_bmp()
{
    global $db, $dbtable_bmp, $ipRemote, $portRemote, $dateCreate;
    $errors = 0;

    if ( isset($_GET["t"]) )    {
        $t = mysqli_real_escape_string($db, $_GET["t"]);
        if ( is_numeric($t) ) {
            $temperature = floatval($t);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["t"];
            $temperature = 0;
            $errors++;
        }
    } else {
        $temperature = 0;
        $errors++;
    }

    if ( isset($_GET["p"]) )    {
        $p = mysqli_real_escape_string($db, $_GET["p"]);
        if ( is_numeric($p) ) {
            $pressure = floatval($p);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["p"];
            $pressure = 0;
            $errors++;
        }
    } else {
        $pressure = 0;
        $errors++;
    }

    if ( isset($_GET["s"]) )    {
        $s = mysqli_real_escape_string($db, $_GET["s"]);
        if ( is_numeric($s) ) {
            $idSensor = floatval($s);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["s"];
            $idSensor = 0;
            $errors++;
        }
    } else {
        $idSensor = 0;
        $errors++;
    }

    if ( isset($_GET["m"]) )    {
        $m = mysqli_real_escape_string($db, $_GET["m"]);
        if ( is_numeric($m) ) {
            $millis = intval($m);
        } else {
            echo "0\n", "ERROR. Not numeric: ", $_GET["m"];
            $millis = 0;
            $errors++;
        }
    } else {
        $millis = 0;
        $errors++;
    }

    if ( (null == $temperature) && (null == $pressure) && (null == $idSensor ) )  {
        echo "0\n", "WARNING. All value is NULL.";
        $errors++;
    }

    // запись данных таблицу BMP датчика
    $query = "INSERT INTO $dbtable_bmp (ipRemote, idSensor, dateCreate, temperature, pressure, millis, errors) ".
            ' VALUES ('. $ipRemote .", '". $idSensor ."', '". $dateCreate ."', ". $temperature .', '. $pressure .', '. $millis .', '. $errors .');';

    // debug
    //echo $query; echo '<br><br>'; exit;

    if ( !mysqli_query($db, $query) )   {
        echo "0\n", "ERROR. mysqli_query: ", mysqli_sqlstate($db), '<br />', mysqli_error($db);
        my_error_log( __FUNCTION__ .' ERROR. mysqli_sqlstate: ' . mysqli_sqlstate($db) .' mysqli_error: '. mysqli_error($db) );
        @ mysqli_close($db);
        exit();
    }   else {
        my_error_log( __FUNCTION__ ." OK" );
    }
}



/*************************************************************************
 * Главная программа
 **************************************************************************/

// определяем тип структуры данных
if ( isset($_GET["type"]) )    {
    $date_type = $_GET["type"];
} else {
    echo "0\n", "ERROR. Data type not present.";
    my_error_log("ERROR. Data type not present.");
    exit();
}

// в зависимости от типа структуры данных, сохраняем их в соответствующей таблице
switch ($date_type) {
    case 'dht':
        save_dht();
        break;
    case 'bmp':
        save_bmp();
        break;
    default:
        echo "0\n", "ERROR. Wrong data type.";
        my_error_log("ERROR. Wrong data type.");
        exit();
        break;
}

if ( !mysqli_close($db) )	{
    echo "0\n", "ERROR. mysqli_close";
    exit();
}

echo "1\n", "PHP OK";

?>