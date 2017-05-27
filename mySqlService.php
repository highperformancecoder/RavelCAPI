<?php
# © Ravelation 2017
# Released under MIT open source license

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
          if ($row['Field'] != "id$" && $row['Field'] != "value$")
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
             $reductions[$axis]="sum(value$)";
             break;
           case 'prod':
 # SQL doesn't have a product function, so we must fake it using
 # prod(x)=exp(sum(log(x))), taking into account zero and negative values
             $reductions[$axis]="if (value$, if(sum(value$<0)%2, -1, 1) * exp(sum(log(abs(value$)))), 0)";
             break;
           case 'avg':
             $reductions[$axis]="avg(value$)";
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
   
   $query="select value$ from (".$query.") as tmp";
   #echo $query."\n";
   $result=$mysqli->query($query);
   $retval=array();
   while($row = mysqli_fetch_array($result))
   {
     array_push($retval,(float)$row[0]);
   }
   echo json_encode($retval);
}

function doAllData($table)
{
   global $mysqli;
   # get column names
   $result=$mysqli->query("show columns from ".$table);
   $columns=array();
   while($row = mysqli_fetch_array($result))
   {
     if ($row['Field'] != "id$" && $row['Field'] != "value$")
       array_push($columns,$row['Field']);
   }

   # now prepare column ordering so as to serve data back to client in 
   $offsets=array();
   $stride=1;
   foreach ($columns as $col)
   {
      $result=$mysqli->query("select distinct `".$col."` from ".$table);
      $count=0;
      $colOffsets=array();
      $offs=0;
      while($row = mysqli_fetch_array($result))
      {
        $colOffsets[$row[0]]=$offs;
        $offs+=$stride;
        $count++;
      }
      $stride*=$count;
      array_push($offsets,$colOffsets);
   }
   #var_dump($offsets);

   # now grab the data, ordered alphanumerically by the axis labels
   $dbreq="select ".implode(",",$columns).",value$ from ".$table;
   $result=$mysqli->query($dbreq);

   # prepare return array with NANs to indicate missing data
   $cnt=0;
   while($row = mysqli_fetch_array($result,MYSQLI_NUM))
   {
     $idx=0;
     for ($i=0; $i<count($row)-1; ++$i)
        $idx+=$offsets[$i][$row[$i]];
     if ($cnt++==0) {
       print "[";
     }
     else {
       print ",";
     }
     echo $idx,",",(float)$row[count($row)-1];
   }
   print "]";
}

function doAllDataWithSchema($table)
{
   header('Content-type: application/data');
   global $mysqli;               
   # get column names
   $result=$mysqli->query("show columns from ".$table);
   $columns=array();
   while($row = mysqli_fetch_array($result))
   {
     if ($row['Field'] != "id$" && $row['Field'] != "value$")
       array_push($columns,$row['Field']);
   }

   print "{\"dimensions\":[";

   # now prepare column ordering so as to serve data back to client in 
   $offsets=array();
   $stride=1;
   foreach ($columns as $col)
   {
      if ($stride>1) print ",";
      print "{\"axis\":\"$col\",\"slice\":[";
      $result=$mysqli->query("select distinct `".$col."` from ".$table);
      $count=0;
      $colOffsets=array();
      $offs=0;
      while($row = mysqli_fetch_array($result))
      {
        if ($count>0) print ",";
        print "\"$row[0]\"";
        $colOffsets[$row[0]]=$offs;
        $offs+=$stride;
        $count++;
      }
      $stride*=$count;
      array_push($offsets,$colOffsets);
      print "]}";
   }
   print "],\"data\":[";
   
   #var_dump($offsets);

   # now grab the data, ordered alphanumerically by the axis labels
   $dbreq="select ".implode(",",$columns).",value$ from ".$table;
   $result=$mysqli->query($dbreq);

   # prepare return array with NANs to indicate missing data
   $cnt=0;
   while($row = mysqli_fetch_array($result,MYSQLI_NUM))
   {
     $idx=0;
     for ($i=0; $i<count($row)-1; ++$i)
        $idx+=$offsets[$i][$row[$i]];
     if ($cnt++>0)
            print ",";       
     echo $idx,",",(float)$row[count($row)-1];
   }
   print "]}";
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
    case "allData":     
      doAllData($pathInfo[2]);
      break;
    case "allDataWithSchema":     
      doAllDataWithSchema($pathInfo[2]);
      break;
  }
}
?>

