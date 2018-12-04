
proc assert {x args} {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed: $args"
        puts stderr [info frame -1 ]
        exit 1
    }
}
