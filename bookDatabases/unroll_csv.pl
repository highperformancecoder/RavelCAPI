# skip header
for ($i=0; $i<5; ++$i) {$_=<>;}
chomp;
@colName=split/,/;

$naxes=5;


while (<>)
{
    chomp;
    @row=split/,/;
    
    for ($i=$naxes; $i<=$#row; ++$i)
    {
        if ($row[$i] ne '""')
        {
            for ($j=1; $j<$naxes-1; ++$j)
            {
                print "$row[$j],";
            }
            print "$colName[$i],$row[$i]\n";
        }
    }
}
