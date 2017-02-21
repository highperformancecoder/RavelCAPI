use Text::CSV;
$csv=Text::CSV->new({ binary => 1 } );

# skip header
for ($i=0; $i<6; ++$i) {$_=<>;}
chomp;
$csv->parse($_);
@colName=$csv->fields();

$naxes=5;


while (<>)
{
    chomp;
    $csv->parse($_);
    @row=$csv->fields();
    
    for ($i=$naxes; $i<=$#row; ++$i)
    {
        if ($row[$i] ne '')
        {
#            if ($row[3] ne "A:All sectors" || $row[4] ne "M:Market value" ||
#                $row[5] ne "USD:US Dollar" || $row[6] ne "A:Adjusted for breaks") {next;}
            for ($j=1; $j<4; ++$j)
            {
                $row[$j]=~s/^.*://;
                print "\"$row[$j]\",";
            }
            print "\"$colName[$i]\",$row[$i]\n";
        }
    }
}
