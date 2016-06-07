<?php
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

define( 'ABSPATH', dirname(__FILE__) . '/' );
require_once( ABSPATH . 'config.php' );



/*
 * Проверка секретного ключа доступа.
 * Прерывает выполнение, если доступ запрещен.
 * Иначе - хакер или Google подробности здесь http://tim4dev.com/2016/02/bolshoj-brat-google/
 */
function checkAccess() {
    global $access_key;
    if ( isset($_GET["k"]) )    {
        $k = $_GET["k"];
        if ( $access_key != $k ) {
            echo "0\n", "ERROR. Unknown key : ", $k;
            my_error_log( "ERROR. Unknown access key" );
            exit();
        }
    } else {
        echo "0\n", "ERROR. Key not found.";
        my_error_log("ERROR. Access key not found.");
        exit();
    }
    my_error_log( "Access key OK" );
}



/*
 * Ведение лог файла
 */
function my_error_log( $msg )    {
    global $ERROR_LOG, $db, $dbtable_error_log, $ipRemote, $dateCreate;

    // выходим если лог не включен
    if ( !$ERROR_LOG )
        return;

    if (empty($msg))
        return;

    $msg = mysqli_real_escape_string($db, $msg);

    // запись лога
    $query = "INSERT INTO $dbtable_error_log (ipRemote, dateCreate, msg) ".
            ' VALUES ('. $ipRemote .", '". $dateCreate ."', '". $msg ."');";

    mysqli_query($db, $query);
}

?>