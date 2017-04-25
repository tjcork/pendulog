<?php
include 'class.dB.php';

// Our database object
$db = new Db();    

// Quote and escape form submitted values
$time = $db -> quote($_POST['Time']);
$yaw = $db -> quote($_POST['Yaw']);
$pitch = $db -> quote($_POST['Pitch']);
$roll = $db -> quote($_POST['Roll']);

// Insert the values into the database
$result = $db -> query("INSERT INTO data (Time,Yaw,Pitch,Roll) VALUES (" .$time.",". $yaw . "," . $pitch . "," . $roll . ")");




/*
$Roll=$_POST["Roll"];

$Pitch=$_POST["Pitch"];

$Yaw=$_POST["Yaw"];

$Write="<p>Roll:  " . $Roll . "</p><p>Pitch:  " . $Pitch . "</p><p>Yaw:  " . $Yaw . "</p>";

file_put_contents('data.html', $Write);
*/

?>
