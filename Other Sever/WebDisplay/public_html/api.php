<?php 

include 'class.dB.php';

// Our database object
$db = new Db();    

$result = $db -> query("SELECT * FROM data WHERE id=". $db -> quote($_POST['id']));                       //fetch result    

  //--------------------------------------------------------------------------
  // 3) echo result as json 
  //--------------------------------------------------------------------------
  echo json_encode($result);

?>
