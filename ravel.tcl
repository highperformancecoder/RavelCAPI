#!ravelTest
#proc savebgerror {} {}
#cli
GUI
toplevel .tl 
wm title .tl Ravel
canvas .tl.ravel -width 500 -height 500
pack .tl.ravel

set palette {black red green blue magenta cyan orange purple}

Ravel ravel
ravel.rescale 50
ravel.x 110
ravel.y 120

proc addHandle {canvas ravel description} {
    global palette
    set handleId [$ravel.addHandle $description  {}]
    $ravel.handles.@elem $handleId
    $canvas create line [ravel.x] [ravel.y] \
        [expr [$ravel.x]+[$ravel.handles($handleId).x]] \
        [expr [$ravel.y]+[$ravel.handles($handleId).y]] \
             -arrow last -tags $ravel$handleId \
             -fill [lindex $palette [expr $handleId%[llength $palette]]]
    $canvas create text 0 0 -text [$ravel.handles($handleId).description] -tag $ravel$handleId.text
    $canvas bind $ravel$handleId <B1-Motion> "moveHandle $canvas $ravel $handleId %x %y"
    $canvas bind $ravel$handleId <ButtonRelease-1> "snapHandle $canvas $ravel $handleId"
}

proc moveLabel {canvas ravel id} {
    set x [$ravel.handles($id).x]
    set y [$ravel.handles($id).y]
    if {$y>0&&abs($x)<abs($y)} {
        $canvas itemconfigure $ravel$id.text -anchor se
    } else {
        $canvas itemconfigure $ravel$id.text -anchor sw
    }
    $canvas coords $ravel$id.text [expr [$ravel.x]+$x] [expr [$ravel.y]+$y]
}

# called after all handles added to the ravel to ensure handle accessors are valid
proc redraw {canvas ravel} {
    for {set i 0} {$i<[$ravel.handles.size]} {incr i} {
        $canvas coords $ravel$i [$ravel.x] [$ravel.y] [expr [$ravel.x]+[$ravel.handles($i).x]] [expr [$ravel.y]+[$ravel.handles($i).y]]
        moveLabel $canvas $ravel $i
    }
}

proc moveHandle {canvas ravel handleId x y} {
    disableEventProcessing
    global discardMotionEvents
    if {$discardMotionEvents} {
        # This piece of merde is necessary because button release
        # generates another motion event
        set discardMotionEvents 0
        return
    }
    $ravel.moveHandleTo $handleId $x $y
    $canvas coords $ravel$handleId [$ravel.x] [$ravel.y] \
        [expr [$ravel.x]+[$ravel.handles($handleId).x]] \
        [expr [$ravel.y]+[$ravel.handles($handleId).y]]
    moveLabel $canvas $ravel $handleId
    enableEventProcessing
}

set discardMotionEvents 0

proc snapHandle {canvas ravel handleId} {
    global discardMotionEvents
    set discardMotionEvents 1
    $ravel.snapHandle $handleId
    redraw $canvas $ravel
} 

addHandle .tl.ravel ravel "x000"
addHandle .tl.ravel ravel "y000"
addHandle .tl.ravel ravel "z000"
addHandle .tl.ravel ravel "t000"
for {set i 0} {$i<[ravel.handles.size]} {incr i} {
    ravel.handles.@elem $i
}
redraw .tl.ravel ravel

.user1 configure -command "redraw .tl.ravel ravel"
