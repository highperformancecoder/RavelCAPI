#!ravelTest
#proc savebgerror {} {}
#cli
GUI
toplevel .tl 
wm title .tl Ravel
canvas .tl.ravel -width 500 -height 500
pack .tl.ravel -expand 1 -fill both

set rr [.tl.ravel create ravel 0 0  -ravelName ravel -tag ravel]

.tl.ravel bind $rr <ButtonPress-1> "ravel.onMouseDown %x %y" 
.tl.ravel bind $rr <B1-Motion> "ravel.onMouseMotion %x %y; redraw" 
.tl.ravel bind $rr <ButtonRelease-1> "ravel.onMouseUp %x %y; redraw" 
.tl.ravel bind $rr <ButtonPress-3> "opMenu %x %y %X %Y" 
.tl.ravel bind $rr <Motion> "ravel.onMouseOver %x %y; redraw"

bind .tl <Configure> "resize %w %h" 

proc resize {w h} {
    set x [expr $w/2]
    set y [expr $h/2]
    .tl.ravel coords ravel $x $y
    ravel.x $x
    ravel.y $y
    ravel.rescale [expr 0.75*min($x,$y)]
}

resize 500 500
ravel.axisMenu.label "axis menu"

ravel.addHandle "x000" {foo bar}
ravel.addHandle "y000" {africa asia america}
ravel.addHandle "z000" {1990 1991 1992}
ravel.addHandle "t000" {a b c}

for {set i 0} {$i<[ravel.handles.size]} {incr i} {
    ravel.handles.@elem $i
}

#force bounding box to be recomputed
.tl.ravel itemconfigure $rr -ravelName ravel
#.tl.ravel create rectangle [.tl.ravel bbox ravel] -outline green

proc redraw {} {
    uplevel #0 {.tl.ravel itemconfigure $rr -ravelName ravel}
}

proc setOp {h op} {
    ravel.handles($h).reductionOp $op
    redraw
}

proc opMenu {x y X Y} {
    set h [ravel.handleIfMouseOverAxisLabel $x $y]
    if {$h>=0} {
        if {![winfo exists .editName]} {
            toplevel .editName 
            wm title .editName "Edit axis descriptor"
            entry .editName.text
            pack .editName.text
        }
        .editName.text configure -validate none
        .editName.text configure -vcmd "ravel.handles($h).description %P; redraw; return 1"
        .editName.text delete 0 end
        .editName.text insert 0 [ravel.handles($h).description]
        .editName.text configure -validate all
        raise .editName.text
        return
    }
    set h [ravel.handleIfMouseOverOpLabel $x $y]
    if {$h>=0} {
        if [winfo exists .opMenu] {
            .opMenu delete 0 end
        } else {
            menu .opMenu
        }
        foreach op {sum prod av stddev min max} {
            .opMenu add command -label $op -command "setOp $h $op"
        }
        tk_popup .opMenu $X $Y
    }
}
