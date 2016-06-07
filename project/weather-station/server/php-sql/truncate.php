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

$db = mysqli_connect($dbhost, $dbuser, $dbpassword, $dbname);

/* check connection */
if ( mysqli_connect_errno() ) {
    echo "0\n", 'ERROR. mysqli_connect_error ', mysqli_connect_error();
    exit();
}

echo "<pre>TRUNCATE $dbtable_dht\n";
mysqli_query($db, "TRUNCATE $dbtable_dht");
echo "TRUNCATE $dbtable_bmp\n";
mysqli_query($db, "TRUNCATE $dbtable_bmp");
mysqli_close($db);

echo "1\n", "OK</pre>";
?>