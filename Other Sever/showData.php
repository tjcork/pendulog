<?php
include 'class.dB.php';

// Our database object
$db = new Db();    




$rows=$db -> select("SELECT * FROM data");
echo 'Time&nbsp;&nbsp;&nbsp;&nbsp;Yaw&nbsp;&nbsp;&nbsp;&nbsp;Pitch&nbsp;&nbsp;&nbsp;&nbsp;Roll<br>';

foreach ($rows as $value) {
   	extract($value);
   	echo $Time.'&nbsp;&nbsp;&nbsp;&nbsp;'. $Yaw .'&nbsp;&nbsp;&nbsp;&nbsp;' . $Pitch . '&nbsp;&nbsp;&nbsp;&nbsp;'. $Roll .'<br>';                
}

?>

