<?php

include 'class.dB.php';


$db = new Db(); 
echo $db -> query('DROP TABLE IF EXISTS data');
