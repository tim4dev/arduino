<?php
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );
require_once( ABSPATH . 'function.php' );

// проверка правомочности доступа
checkAccess();

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
    <head>
        <meta charset="utf-8">
        <meta HTTP-EQUIV="refresh" content="240">
        <title>Weather Station RF24-ESP8266-DHT</title>
    </head>
<body>

<p>Last refresh page:
<?php echo date("Y-m-d H:i:s", time()); ?>
</p>

<?php

$SQL_LIMIT = 1000;

// если установлен параметр all то показать все записи, иначе - только за прошедшие сутки
$all = isset($_GET["all"]);
if ( $all )    {
    echo "<p>Show all data (limit $SQL_LIMIT).  [<a href=\"weather-view.php?k=$access_key\">Show data in the last 24 hours</a>]</p>";
    $query_dht = "SELECT id, INET_NTOA(ipRemote) AS ipRemote, idSensor, dateCreate, temperature, humidity, voltage, millis FROM $dbtable_dht
            ORDER BY dateCreate DESC
            LIMIT $SQL_LIMIT";
    $query_bmp = "SELECT id, idSensor, dateCreate, temperature, pressure, millis FROM $dbtable_bmp
            ORDER BY dateCreate DESC
            LIMIT $SQL_LIMIT";
}   else {
    echo "<p>Show data in the last 24 hours. [<a href=\"weather-view.php?k=$access_key&all\">Show all data (limit $SQL_LIMIT)</a>]</p>";
    $query_dht = "SELECT id, INET_NTOA(ipRemote) AS ipRemote, idSensor, dateCreate, temperature, humidity, voltage, millis FROM $dbtable_dht
              WHERE dateCreate >= NOW() - INTERVAL 1 DAY
              ORDER BY dateCreate DESC";
    $query_bmp = "SELECT id, idSensor, dateCreate, temperature, pressure, millis FROM $dbtable_bmp
            WHERE dateCreate >= NOW() - INTERVAL 1 DAY
            ORDER BY dateCreate DESC";
}

echo '<p> <ol> Charts (last 3 days) :';
echo "<li> Sensor #20 : <a href=\"chart-dht.php?k=$access_key&id=20&t=t\"> Temperature </a>, <a href=\"chart-dht.php?k=$access_key&id=20&t=h\"> Humidity </a>, <a href=\"chart-dht.php?k=$access_key&id=20&t=v\"> Voltage </a> </li>";
echo "<li> Sensor #11 : <a href=\"chart-dht.php?k=$access_key&id=11&t=t\"> Temperature </a>, <a href=\"chart-dht.php?k=$access_key&id=11&t=h\"> Humidity </a>, <a href=\"chart-dht.php?k=$access_key&id=11&t=v\"> Voltage </a>, <a href=\"chart-dht.php?k=$access_key&id=11&t=p\"> Pressure </a> </li>";
echo '</ol> </p>';

echo '<p> <ol> Export data to CSV :';
echo "<li> <a href=\"export-voltage.php?k=$access_key&id=20\"> Voltage Sensor #20 </a> </li>";
echo "<li> <a href=\"export-dht.php?k=$access_key&id=11\">     DHT Sensor #11     </a> </li>";
echo "<li> <a href=\"export-dht.php?k=$access_key&id=20\">     DHT Sensor #20     </a> </li>";
echo '</ol> </p>';

$db = mysqli_connect($dbhost, $dbuser, $dbpassword, $dbname);

/* check connection */
if ( mysqli_connect_errno() ) {
    echo "0\n", 'ERROR. mysqli_connect_error ', mysqli_connect_error();
    exit();
}

/*************************************************************************
 *
 * DHT датчики
 *
 **************************************************************************/

// выборка данных
//echo $query_dht; echo '<br><br>'; exit;

$res = mysqli_query($db, $query_dht);
if ( !$res )
{
    echo "0\n", "ERROR. mysqli_query: ", mysqli_sqlstate($db), '<br />', mysqli_error($db);
    @ mysqli_close($db);
    exit();
}
?>

<div style="float: left; width: 50%; margin-right: 30px;">
<table border="1" style="border-collapse:collapse;border:1px solid FFCC00;color:000000" cellpadding="3" cellspacing="3" style="float: left;">
<caption>DHT sensors</caption>
<tr>
    <th>id</th>
    <th>IP remote</th>
    <th>id Sensor</th>
    <th>date Create</th>
    <th>temperature, ℃</th>
    <th>humidity, %</th>
    <th>voltage</th>
    <th>millis</th>
</tr>

<?php

while($row = mysqli_fetch_array($res)) {
    echo '<tr>';
    echo '<td>', $row["id"], '</td>';
    echo '<td>', $row["ipRemote"], '</td>';
    echo '<td>', $row["idSensor"], '</td>';
    echo '<td>', $row["dateCreate"], '</td>';
    echo '<td>', $row["temperature"], '</td>';
    echo '<td>', $row["humidity"], '</td>';
    echo '<td>', $row["voltage"], '</td>';
    echo '<td>', $row["millis"], '</td>';
    echo '</tr>';
}



/*************************************************************************
 *
 * BMP датчик
 *
 **************************************************************************/
// выборка данных
//echo $query_bmp; echo '<br><br>'; exit;

$res = mysqli_query($db, $query_bmp);
if ( !$res )
{
    echo "0\n", "ERROR. mysqli_query: ", mysqli_sqlstate($db), '<br />', mysqli_error($db);
    @ mysqli_close($db);
    exit();
}
?>
</table>
</div>



<div style="float: left; width: 40%;">
<table border="1" style="border-collapse:collapse;border:1px solid FFCC00;color:000000" cellpadding="3" cellspacing="3">
<caption>BMP sensors</caption>
<tr>
    <th>id</th>
    <th>id Sensor</th>
    <th>date Create</th>
    <th>temperature, ℃</th>
    <th>pressure, hPa<br>standart = 1013 hPa</th>
    <th>millis</th>
</tr>

<?php

while($row = mysqli_fetch_array($res)) {
    echo '<tr>';
    echo '<td>', $row["id"], '</td>';
    echo '<td>', $row["idSensor"], '</td>';
    echo '<td>', $row["dateCreate"], '</td>';
    echo '<td>', $row["temperature"], '</td>';
    echo '<td>', $row["pressure"], '</td>';
    echo '<td>', $row["millis"], '</td>';
    echo '</tr>';
}



mysqli_close($db);

?>
</table>
</div>

</body>
</html>