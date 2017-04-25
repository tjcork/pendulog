<?php 

include 'class.dB.php';

// Our database object
$db = new Db();    
$id = $db -> quote($_GET['id']);


$result = $db -> select("SELECT * FROM data WHERE id=". $id);                       //fetch result

//echo $result;
  //--------------------------------------------------------------------------
  // 3) echo result as json 
  //--------------------------------------------------------------------------
echo json_encode($result);

?>
