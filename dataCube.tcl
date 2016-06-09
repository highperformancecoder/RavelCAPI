#!ravelTest
GUI
toplevel .tl 
wm title .tl Ravel

canvas .tl.ravel -width 1000 -height 500 -closeenough 3
pack .tl.ravel

if {[string equal unix $tcl_platform(platform)]} {
    load libTktable2.11[info sharedlibextension]
} else {
    load Tktable211.dll
}

set palette {black red darkgreen blue magenta cyan orange purple}

DataCube dc
dc.ravel.rescale 150
dc.ravel.x 50
dc.ravel.y 50

#set rr [.tl.ravel create ravel 110 120 -ravelName dc.ravel -tag ravel]

# forces a redraw of the ravel
proc redraw {} {
    global t dataCube
    # clear everything in the spreadsheet
    array unset dataCube
    dc.populateArray dataCube
#    $t delete rows 0 [$t cget -rows]
    dc.render
    update
#    setSpreadsheetDimensions
#    uplevel #0 {.tl.ravel itemconfigure $rr -ravelName dc.ravel}
}


#.tl.ravel bind $rr <ButtonPress-1> "dc.ravel.onMouseDown %x %y"
#.tl.ravel bind $rr <ButtonRelease-1> "dc.ravel.onMouseUp %x %y; redraw"
#.tl.ravel bind $rr <B1-Motion> "dc.ravel.onMouseMotion %x %y; redraw"

.user1 configure -command "redraw .tl.ravel ravel"

frame .tl.ravel.spreadsheet
set t .tl.ravel.spreadsheet.table
table $t \
    -rows 100 \
    -cols 10 \
    -colwidth 20 \
    -titlerows 0 \
    -titlecols 0 \
    -yscrollcommand ".tl.ravel.spreadsheet.sy set" \
    -xscrollcommand ".tl.ravel.spreadsheet.sx set" \
    -coltagcommand colorize \
    -flashmode off \
    -selectmode extended \
    -ellipsis on \
    -width 20 -height 20 \
    -colstretch all \
    -rowstretch all \
    -multiline 0 \
    -variable dataCube

scrollbar .tl.ravel.spreadsheet.sy -command [list $t yview]
scrollbar .tl.ravel.spreadsheet.sx -command [list $t xview] -orient horizontal
pack .tl.ravel.spreadsheet.sy -side right -fill y
pack .tl.ravel.spreadsheet.table -fill both 
pack .tl.ravel.spreadsheet.sx -fill x

#.tl.ravel create window [expr [dc.ravel.x]+2] [expr [dc.ravel.y]+2] -window .tl.ravel.spreadsheet -anchor nw 
.tl.ravel create window  0 50 -window .tl.ravel.spreadsheet -anchor nw 

button .tl.ravel.open -command openFile -text "Open"
.tl.ravel create window 300 30 -window .tl.ravel.open

toplevel .csvForm
frame .csvForm.separator
radiobutton .csvForm.separator.comma -text "comma" -value "," \
    -variable separator -command populateFormTable 
radiobutton .csvForm.separator.space -text "space" -value " " \
    -variable separator -command populateFormTable 
radiobutton .csvForm.separator.tab -text "tab" -value "\t" \
    -variable separator -command populateFormTable 

frame .csvForm.headers

frame .csvForm.headers.header
ttk::spinbox .csvForm.headers.header.val -from 0  -to 100000 -increment 1 -command populateFormTable
 .csvForm.headers.header.val set 0
label .csvForm.headers.header.text -text "header lines"
pack .csvForm.headers.header.text .csvForm.headers.header.val

frame .csvForm.headers.colHeader
ttk::spinbox .csvForm.headers.colHeader.val -from 0 -to 1 -increment 1  -command populateFormTable
.csvForm.headers.colHeader.val set 1
label .csvForm.headers.colHeader.text -text "column header lines"
pack .csvForm.headers.colHeader.text .csvForm.headers.colHeader.val

frame .csvForm.headers.rowDescriptors
ttk::spinbox .csvForm.headers.rowDescriptors.val -from 0 -to 100000 -increment 1  -command populateFormTable
.csvForm.headers.rowDescriptors.val set 1
label .csvForm.headers.rowDescriptors.text -text "dimension columns"
pack .csvForm.headers.rowDescriptors.text .csvForm.headers.rowDescriptors.val

pack .csvForm.separator.comma .csvForm.separator.space .csvForm.separator.tab \
    -side left
#pack .csvForm.headers.header  .csvForm.headers.colHeader  \
#    .csvForm.headers.rowDescriptors -side left

table .csvForm.table -rows 5 -cols 5 -colwidth 20 -variable csvFormData

frame .csvForm.buttonBar
button .csvForm.buttonBar.cancel -text cancel -command {grab release .csvForm; wm withdraw .csvForm}
button .csvForm.buttonBar.ok -text OK -command {grab release .csvForm; wm withdraw .csvForm; loadData}
pack .csvForm.buttonBar.cancel .csvForm.buttonBar.ok -side left

pack .csvForm.separator .csvForm.headers .csvForm.table .csvForm.buttonBar
wm withdraw .csvForm

proc openFile {} {
    global startData fname separator
    set fname [tk_getOpenFile]
    if [string length $fname] {
        set f [open "$fname"]
        set startData ""
        for {set i 0} {![eof $f] && $i<5} {incr i} {
            lappend startData [gets $f]
        }
        close $f
        dc.separator $separator
        dc.initSpec $fname
        populateFormTable
        wm deiconify .csvForm
        grab set .csvForm
    }
}

set separator ","
proc populateFormTable {} {
    global separator startData csvFormData
    array unset csvFormData
    .csvForm.table clear tags
    dc.separator $separator

    for {set i 0} {$i<[dc.nColAxes]} {incr i} {
        .csvForm.table tag col axis $i
    }
    for {set i 0} {$i<[dc.nRowAxes]} {incr i} {
        .csvForm.table tag row axis $i
    }
    foreach r [dc.commentRows] {
        .csvForm.table tag row delete $r
        .csvForm.table tag row comment $r
    }
    foreach c [dc.commentCols] {
        .csvForm.table tag col delete $c
        .csvForm.table tag col comment $c
    }

    .csvForm.table tag configure comment -fg red
    .csvForm.table tag configure axis -fg blue
    for {set i 0} {$i<[llength $startData]} {incr i} {
        set line [split [lindex $startData $i] $separator]
        for {set j 0} {$j<5 && $j<[llength $line]} {incr j} {
            set csvFormData($i,$j) [lindex $line $j]
        }
    }
}

proc setSpreadsheetDimensions {} {
    set xh [dc.ravel.handles.@elem [dc.ravel.xHandleId]]
    set yh [dc.ravel.handles.@elem [dc.ravel.yHandleId]]
    .tl.ravel.spreadsheet.table configure \
        -rows [expr [llength [$yh.sliceLabels]]+1] \
        -cols [expr [llength [$xh.sliceLabels]]+1]
}

proc loadData {} {
    global fname nDdata dataCube
    dc.loadFile $fname
#    setSpreadsheetDimensions 
    array unset dataCube
    dc.populateArray dataCube
    redraw 
}
 
# removes elem from set if it exists, otherwise adds it
proc toggleSetElem {Set elem} {
    set ret {}
    foreach e $Set {
        if {$e!=$elem} {lappend ret $e}
    }
    if {[llength $ret]==[llength $Set]} {
        # need to add elem
        lappend ret $elem
    }
    return $ret
}

           
proc toggleColComment {col} {
    dc.commentCols [toggleSetElem [dc.commentCols] $col]
    populateFormTable
}
proc toggleRowComment {row} {
    dc.commentRows [toggleSetElem [dc.commentRows] $row]
    populateFormTable
}
proc setnColAxes {col} {
    dc.nColAxes [expr $col+1]
    populateFormTable
}
proc setnRowAxes {row} {
    dc.nRowAxes [expr $row+1]
    populateFormTable
 }

menu .rowColContext
bind .csvForm.table <Button-3> {
    set row [.csvForm.table index @%x,%y row]
    set col [.csvForm.table index @%x,%y col]
    if {$row==0 && $col==0} {
        .rowColContext delete 1 end
        .rowColContext add command -label "set column axes" -command "setnColAxes $col"
        .rowColContext add command -label "set row axes" -command "setnRowAxes $row"
        .rowColContext add command -label "toggle column comment" -command "toggleColComment $col"
        .rowColContext add command -label "toggle row comment" -command "toggleRowComment $row"
        tk_popup .rowColContext %X %Y
        
    } elseif {$row==0} {
        .rowColContext delete 1 end
        .rowColContext add command -label "set column axes" -command "setnColAxes $col"
        .rowColContext add command -label "toggle comment" -command "toggleColComment $col"
        tk_popup .rowColContext %X %Y
    } elseif {$col==0} {
        .rowColContext delete 1 end
        .rowColContext add command -label "set row axes" -command "setnRowAxes $row"
        .rowColContext add command -label "toggle comment" -command "toggleRowComment $row"
        tk_popup .rowColContext %X %Y
    }
}

.user1 configure -text Ravel -command {
    toplevel .ravelFilter
    dc.width 300
    dc.height 300
    image create photo filterImage -width [dc.width] -height [dc.height]
    image create photo ravelImage -width [expr int(2.1*[dc.ravel.radius])] \
        -height [expr int(2.1*[dc.ravel.radius])]
    dc.ravel.x [expr 0.5*[image width ravelImage]]
    dc.ravel.y [expr 0.5*[image height ravelImage]]
    label .ravelFilter.filter -image filterImage
    label .ravelFilter.ravel -image ravelImage
    pack .ravelFilter.ravel .ravelFilter.filter -side left
    dc.setFilterImage filterImage
    dc.setRavelImage ravelImage
    bind .ravelFilter.filter <ButtonPress-1> {dc.onMouseDown %x %y}
    bind .ravelFilter.filter <ButtonRelease-1> {dc.onMouseUp %x %y; redraw}
    bind .ravelFilter.filter <Motion> {dc.onMouse %x %y}
    bind .ravelFilter.ravel <ButtonPress-1> {dc.ravel.onMouseDown %x %y}
    bind .ravelFilter.ravel <ButtonRelease-1> {dc.ravel.onMouseUp %x %y; redraw}
    bind .ravelFilter.ravel <B1-Motion> {dc.ravel.onMouseMotion %x %y; dc.render}
    bind .ravelFilter.ravel <Motion> {dc.ravel.onMouseOver %x %y; dc.render}
}
