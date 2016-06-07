
-- см. также config.php

-- создать БД
CREATE DATABASE IF NOT EXISTS db_weather;

-- создать пользователя
CREATE USER  'u_weather'@'localhost' IDENTIFIED BY '***PASSWORD***';
GRANT ALL ON db_weather.* TO 'u_weather'@'localhost';
