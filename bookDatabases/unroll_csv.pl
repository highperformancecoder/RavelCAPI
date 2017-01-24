use Text::CSV;
$csv=Text::CSV->new({ binary => 1 } );

# skip header
for ($i=0; $i<6; ++$i) {$_=<>;}
chomp;
$csv->parse($_);
@colName=$csv->fields();

$naxes=6;


while (<>)
{
    chomp;
    $csv->parse($_);
    @row=$csv->fields();
    
    for ($i=$naxes; $i<=$#row; ++$i)
    {
        if ($row[$i] ne '')
        {
            for ($j=1; $j<$naxes-1; ++$j)
            {
                print "\"$row[$j]\",";
            }
            print "\"$colName[$i]\",$row[$i]\n";
        }
    }
}
