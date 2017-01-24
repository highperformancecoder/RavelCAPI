<?php
#echo "<pre>";
#var_dump($_SERVER);

require "./credentials.php";

$mysqli = new mysqli("localhost", $username, $password, "hpcoders_BIS");

if ($mysqli->connect_errno) {
    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}



# axes/<table>
# axes/<table>/<axisname>
function doAxes($pathInfo)
{
  global $mysqli;
  $retval=array();
  switch (count($pathInfo))
  {
      case 2: # get table names
        $result=$mysqli->query("show tables");
        while($row = mysqli_fetch_array($result))
        array_push($retval,$row[0]);
        break;

      case 3: # get column names
        $result=$mysqli->query("show columns from ".$pathInfo[2]);
        while($row = mysqli_fetch_array($result))
        {
          if ($row['Field'] != "id" && $row['Field'] != "value")
            array_push($retval,$row['Field']);
        }
        break;
      case 4: # get labels
        $result=$mysqli->query("select distinct `".$pathInfo[3]."` from ".$pathInfo[2]);
        while($row = mysqli_fetch_array($result))
          array_push($retval,$row[0]);
        break;
}
  
  echo json_encode($retval);
}

function addWhere(&$where,$clause)
{
  if ($where!="")
    $where.=" and ";
  $where.=$clause;  
}

function doData($table)
{
   global $mysqli;
   $where="";
   $cols="";
   $reducedcols=array();
   $reductions=array();
   foreach ($_GET as $axis => $expr)
   {
     if ($expr=="")
       {
         if ($cols!="") $cols.=",";
         $cols.="`".$axis."`";
       }
     else if (preg_match("/slice\((.*)\)/",$expr,$args))
       addWhere($where,$axis."='".$args[1]."'");
     else if (preg_match("/reduce\((.*)\)/",$expr,$args))
     {
       array_push($reducedcols,$axis);
       switch ($args[1])
         {
           case 'sum':
             $reductions[$axis]="sum(value)";
             break;
           case 'prod':
 # SQL doesn't have a product function, so we must fake it using
 # prod(x)=exp(sum(log(x))), taking into account zero and negative values
             $reductions[$axis]="if (value, if(sum(value<0)%2, -1, 1) * exp(sum(log(abs(value)))), 0)";
             break;
           case 'avg':
             $reductions[$axis]="avg(value)";
             break;
          }
     }
     else if (preg_match("/filterValue\((.*),(.*)\)/",$expr,$args)) {}
     else if (preg_match("/filter\((.*),(.*)\)/",$expr,$args))
       {
         # a more complicated example where we want all values in the
         # range axis label1 to label2, in database order.
         addWhere($where,$axis." in (");
         # need to grab the axis labels
         $result=$mysqli->query("select distinct `".$axis."` from ".$table);
         $flag=false;
         while($row = mysqli_fetch_array($result))
           {
             if ($flag)
               $where.=',';
             if ($row[0]==$args[0])
               $flag=true;
             if ($flag)
               $where.="'".$row[0]."'";
             if ($row[0]==$args[1])
               $flag=false;
           }
         $where.=")";
       }
      
   }

   # now build up the query statement, reduction by reduction
   $query="select * from $table where $where";
   while ($r=array_pop($reducedcols))
   {
     # append all remaining reduced cols to column vector
     $c=$cols;
     foreach ($reducedcols as $i) $c.=",$i";
     $query="select $reductions[$r] as value,$c from ($query) as t$r group by $c"; 
   }
   
   $query="select value from (".$query.") as tmp";
   #echo $query."\n";
   $result=$mysqli->query($query);
   $retval=array();
   while($row = mysqli_fetch_array($result))
   {
     array_push($retval,(float)$row[0]);
   }
   echo json_encode($retval);
}

$pathInfo=explode("/",$_SERVER["PATH_INFO"]);
if (count($pathInfo)>1)
{
  switch($pathInfo[1])
  {
    case "data":
      doData($pathInfo[2]);
      break;
    case "axes":
      doAxes($pathInfo);
      break;
  }
}
?>

