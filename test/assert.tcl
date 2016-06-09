
proc assert {x args} {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed: $args"
        exit 1
    }
}
