<?php
echo "<pre>";
#var_dump($_SERVER);
#include "credentials.php";
$username="ravel";
$password="ravel";

$mysqli = new mysqli("localhost", $username, $password, "BIS");

if ($mysqli->connect_errno) {
    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}
#echo $mysqli->host_info . "\n";

function doAxes($pathInfo)
{
  global $mysqli;
  $retval=array();
  switch (count($pathInfo))
  {
      case 2: # get column names
        $result=$mysqli->query("show columns from ".$_GET["table"]);
        while($row = mysqli_fetch_array($result))
        {
          if ($row['Field'] != "id" && $row['Field'] != "value")
            array_push($retval,$row['Field']);
        }
        break;
      case 3: # get labels
        $result=$mysqli->query("select distinct `".$pathInfo[2]."` from ".$_GET["table"]);
        while($row = mysqli_fetch_array($result))
          array_push($retval,$row[0]);
        break;
}
  
  echo json_encode($retval);
}

$pathInfo=explode("/",$_SERVER["PATH_INFO"]);
#var_dump($pathInfo);
if (count($pathInfo)>1)
{
  switch($pathInfo[1])
  {
    case "data":
      break;
    case "axes":
      doAxes($pathInfo);
     break;
  }
}
?>

