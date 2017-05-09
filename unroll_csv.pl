use Text::CSV;
$csv=Text::CSV->new({ binary => 1 } );

# skip header
for ($i=0; $i<1; ++$i) {$_=<>;}
chomp;
$csv->parse($_);
@colName=$csv->fields();

$naxes=2;


while (<>)
{
    chomp;
    $csv->parse($_);
    @row=$csv->fields();
    
    for ($i=$naxes; $i<=$#row; ++$i)
    {
        if ($row[$i] ne '')
        {
            for ($j=0; $j<$naxes; ++$j)
            {
                print "\"$row[$j]\",";
            }
            print "\"$colName[$i]\",$row[$i]\n";
        }
    }
}
