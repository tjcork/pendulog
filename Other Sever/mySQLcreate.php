<?php
include 'class.dB.php';
$precision='12';
$db=new dB();

// Create database
$db -> query("CREATE DATABASE IF NOT EXISTS myDB");

echo $db -> query( "CREATE TABLE IF NOT EXISTS data(
id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY, 
Time INT,
Roll FLOAT(".$precision."),
Pitch FLOAT(".$precision."),
Yaw FLOAT(".$precision.")
)");