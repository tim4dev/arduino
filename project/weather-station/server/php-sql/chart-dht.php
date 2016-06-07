<html>
<?php
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

// https://developers.google.com/chart/interactive/docs/gallery/linechart
// https://developers.google.com/chart/interactive/docs/datesandtimes


define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );
require_once( ABSPATH . 'function.php' );

// проверка правомочности доступа
checkAccess();

if ( ! isset($_GET["id"]) )  {
    echo "<pre>idSensor not found. Exit()</pre>";
    exit();
}
$idSensor = intval($_GET["id"]);

if ( ! isset($_GET["t"]) )  {
    echo "<pre>DHT type not found. Exit()</pre>";
    exit();
}

define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );

/* Что выводить:
 * t - температура
 * h - влажность
 * v - напряжение питания
 * p - атмосферное давление
 */
$dht_type = addslashes($_GET["t"]);

switch ($dht_type)  {
    case 't':
        $dbtable = $dbtable_dht;
        $query_part = "temperature";
        break;
    case 'h':
        $dbtable = $dbtable_dht;
        $query_part = "humidity";
        break;
    case 'v':
        $dbtable = $dbtable_dht;
        $query_part = "voltage";
        break;
    case 'p':
        $dbtable = $dbtable_bmp;
        $query_part = "pressure";
        break;
    default:
        echo "<pre>DHT type not valid. Exit()</pre>";
        exit();
        break;
}

// берем LIMIT последних записей
$query_dht = "SELECT dateCreate, $query_part FROM (
                SELECT dateCreate, $query_part
                FROM $dbtable
                WHERE (idSensor = $idSensor) AND ((errors = 0) OR (errors IS NULL))
                ORDER BY dateCreate DESC LIMIT 1000
            ) sub
            ORDER BY dateCreate ASC";

/*"SELECT dateCreate, $query_part
            FROM $dbtable
            WHERE (idSensor = $idSensor) AND ((errors = 0) OR (errors IS NULL)) AND ( dateCreate >= NOW() - INTERVAL 3 DAY )
            ORDER BY dateCreate";*/

$db = mysqli_connect($dbhost, $dbuser, $dbpassword, $dbname);

/* check connection */
if ( mysqli_connect_errno() ) {
    echo "0\n", 'ERROR. mysqli_connect_error ', mysqli_connect_error();
    exit();
}

//echo $query_dht; echo "\r\n"; exit; // !!! debug

$res = mysqli_query($db, $query_dht);
if ( !$res )
{
    echo "0\n", "ERROR. mysqli_query: ", mysqli_sqlstate($db), '<br />', mysqli_error($db);
    @ mysqli_close($db);
    exit();
}
?>
  <head>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
      google.charts.load('current', {'packages':['corechart']});
      google.charts.setOnLoadCallback(drawChart);

      function drawChart() {
        var data = google.visualization.arrayToDataTable([
          ['Date', '<?php echo $query_part; ?>'],
<?php
while($row = mysqli_fetch_array($res)) {
    echo "[new Date('", $row["dateCreate"], "'), ", $row[1], '],', "\n";
}
mysqli_close($db);
?>
        ]);

        var options = {
          title: 'Sensor #<?php echo $idSensor, ' <', $query_part, '>'; ?>',
          curveType: 'function',
          legend: { position: 'top' },
          hAxis: {format: 'dd MMM, HH:mm'}
        };

        var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));

        chart.draw(data, options);
      }
    </script>
  </head>
  <body>
    <div id="curve_chart" style="width: 90%; height: 90%"></div>
  </body>
</html>