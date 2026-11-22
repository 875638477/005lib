# OrCAD Capture 17.2. Source this file, then call rc_text_fix.
namespace eval ::CaptureRCFix {
    variable directory [file dirname [file normalize [info script]]]
}
source [file join $::CaptureRCFix::directory capture_rc_text_check.tcl]

proc ::CaptureRCFix::font {caption state} {
    set font [DboTclHelper_sMakeLOGFONT]
    set name [DboTclHelper_sMakeCString {}]
    try {
        $caption GetFont [::CaptureRCText::read $caption GetDefaultFontType $state] $font
        DboTclHelper_sGetLFName $font $name
        set values [list [DboTclHelper_sGetConstCharPtr $name]]
        foreach field {Height Width Escapement Orientation Weight Italic Underline StrikeOut CharSet OutPrecision ClipPrecision Quality PitchAndFamily} {
            lappend values [DboTclHelper_sGetLF$field $font]
        }
        return $values
    } finally {
        DboTclHelper_sDeleteCString $name
        if {[llength [info commands DboTclHelper_sDeleteLOGFONT]]} {DboTclHelper_sDeleteLOGFONT $font}
    }
}

proc ::CaptureRCFix::rectangle {rect} {
    return [concat [::CaptureRCText::point [DboTclHelper_sGetCRectTopLeft $rect]] \
        [::CaptureRCText::point [DboTclHelper_sGetCRectBottomRight $rect]]]
}

proc ::CaptureRCFix::partBox {part state} {
    set cache [::CaptureRCText::read $part GetPart $state]
    lassign [rectangle [$cache GetBoundingBox]] a b c d
    set xs {}; set ys {}
    foreach point [list [list $a $b] [list $c $b] [list $a $d] [list $c $d]] {
        lassign [::CaptureRCText::point [$part GetOffsetGraphicPoint [DboTclHelper_sMakeCPoint {*}$point]]] x y
        lappend xs $x
        lappend ys $y
    }
    set xs [lsort -integer $xs]
    set ys [lsort -integer $ys]
    return [list [lindex $xs 0] [lindex $ys 0] [lindex $xs end] [lindex $ys end]]
}

proc ::CaptureRCFix::boxRow {out sheet type bbox} {
    ::CaptureRCText::writeRow $out [concat [list BOX $sheet $type] $bbox]
}

proc ::CaptureRCFix::integerGrid {value} {
    if {![string is double -strict $value]} {return 0}
    if {[catch {
        expr {$value > 0 && $value < Inf && abs($value - round($value)) < 1e-8 ? round($value) : 0}
    } normalized]} {return 0}
    return $normalized
}

proc ::CaptureRCFix::pinPitchInDocumentUnits {page} {
    # PinToPin is stored in micrometres and the page scale in document units/inch.
    return [expr {[$page GetPinToPin] * [$page GetDocUnitsPerInch] / 25400.0}]
}

proc ::CaptureRCFix::gridSetting {override} {
    if {$override ne ""} {
        set grid [integerGrid $override]
        if {!$grid} {error "The explicit grid must be a positive whole number in Capture document units, for example: rc_text_fix 5"}
        return [list fixed $grid]
    }
    set ratio 0.5
    # Capture returns 0 when focus is in the Project Manager instead of a sheet.
    if {![catch {
        set uiGrid [integerGrid [capGetGridSpacing]]
        if {!$uiGrid} {error "No active grid"}
        set activePitch [pinPitchInDocumentUnits [GetActivePage]]
        if {$activePitch <= 0} {error "Unknown active pin pitch"}
        set ratio [expr {$uiGrid / $activePitch}]
    }]} {
        return [list pitch $ratio]
    }
    puts "No active schematic grid. Using half pin-to-pin on each page."
    return [list pitch 0.5]
}

proc ::CaptureRCFix::page {sch pg number out state refPattern gridSetting} {
    set page [::CaptureRCText::stringField $pg GetName]
    set size [$pg GetDocUnits 0]
    lassign $gridSetting basis grid
    if {$basis eq "pitch"} {
        set grid [integerGrid [expr {[pinPitchInDocumentUnits $pg] * $grid}]]
    }
    ::CaptureRCText::writeRow $out [list PAGE $number $page [$pg GetDocUnitsPerInch] \
        [DboTclHelper_sGetCSizeX $size] [DboTclHelper_sGetCSizeY $size] $grid]
    set bindings {}
    foreach object [::CaptureRCText::objects $pg $state NewPartInstsIter NextPartInst delete_DboPagePartInstsIter] {
        set part [DboPartInstToDboPlacedInst $object]
        if {$part eq "NULL"} {
            boxRow $out $number block [rectangle [$object GetBoundingBox]]
            continue
        }
        set ref [::CaptureRCText::stringField $part GetReferenceDesignator]
        set symbol [::CaptureRCText::stringField $part GetSourcePartName]
        set id [::CaptureRCText::read $part GetId $state]
        lassign [::CaptureRCText::point [::CaptureRCText::read $part GetLocation $state]] ox oy
        set leads [::CaptureRCText::leads $part $state]
        set axis [::CaptureRCText::axisForPins $leads]
        set target [expr {[::CaptureRCText::isCandidate $ref $symbol] && [string match -nocase $refPattern $ref]}]
        set locked [$part IsObjLocked]
        if {$target} {
            ::CaptureRCText::writeRow $out [concat [list PART $number $id $ref $locked $ox $oy] $axis]
        }
        boxRow $out $number body [partBox $part $state]
        set pinLabels [expr {[::CaptureRCText::read $part GetPinNumbersAreVisible $state] ||
                             [::CaptureRCText::read $part GetPinNamesAreVisible $state]}]
        foreach lead $leads {
            lassign $lead pin start hot
            boxRow $out $number [expr {$pinLabels ? "labelled_pin" : "pin"}] [concat $start $hot]
        }
        foreach dp [::CaptureRCText::objects $part $state NewDisplayPropsIter NextProp delete_DboDisplayPropsIter] {
            set type [::CaptureRCText::read $dp GetDisplayType $state]
            if {$type == $::DboValue_DO_NOT_DISPLAY} {continue}
            set did [::CaptureRCText::read $dp GetId $state]
            set name [::CaptureRCText::stringField $dp GetName]
            set value [::CaptureRCText::stringField $dp GetValueString]
            set angle [::CaptureRCText::read $dp GetRotation $state]
            lassign [::CaptureRCText::point [::CaptureRCText::read $dp GetLocation $state]] x y
            set justification [$dp GetHorizontalTextJustification]
            set renderedFont {}
            catch {set renderedFont [font $dp $state]}
            ::CaptureRCText::writeRow $out [concat [list TEXT $number $id $did $name $value $type $angle \
                [expr {$ox+$x}] [expr {$oy+$y}] $justification] $renderedFont]
            if {$target && !$locked && [llength $axis] && $type == $::DboValue_VALUE_ONLY &&
                [string tolower $name] in {{part reference} refdes value user_value}} {
                dict set bindings [list $number $did] [list $dp $part $sch $pg $x $y $angle]
            }
        }
    }
    foreach wire [::CaptureRCText::objects $pg $state NewWiresIter NextWire delete_DboPageWiresIter] {
        set pts {}
        for {set i 0} {$i < [::CaptureRCText::read $wire GetPointCount $state]} {incr i} {
            set p [::CaptureRCText::point [$wire GetPoint $i $state]]
            if {[$state Failed]} {error "Capture could not read a wire point on $page"}
            if {[llength $pts]} {boxRow $out $number wire [concat [lindex $pts end] $p]}
            lappend pts $p
        }
        foreach alias [::CaptureRCText::objects $wire $state NewAliasesIter NextAlias delete_DboWireAliasesIter] {
            set bounds [rectangle [$alias GetBoundingBox]]
            if {[lindex $bounds 0] == [lindex $bounds 2] || [lindex $bounds 1] == [lindex $bounds 3]} {
                lassign [::CaptureRCText::point [::CaptureRCText::read $alias GetLocation $state]] x y
                set width [expr {9 * [string length [::CaptureRCText::stringField $alias GetName]] + 5}]
                set bounds [list [expr {$x-3}] [expr {$y-15}] [expr {$x+$width}] [expr {$y+15}]]
                if {[::CaptureRCText::read $alias GetRotation $state] % 2} {
                    set bounds [list [expr {$x-15}] [expr {$y-$width}] [expr {$x+15}] [expr {$y+$width}]]
                }
            }
            boxRow $out $number alias $bounds
        }
    }
    foreach {new next free kind} {
        NewGlobalsIter NextGlobal delete_DboPageGlobalsIter endpoint
        NewOffPageConnectorsIter NextOffPageConnector delete_DboPageOffPageConnectorsIter endpoint
        NewPortsIter NextPort delete_DboPagePortsIter endpoint
        NewCommentGraphicsIter NextCommentGraphic delete_DboPageCommentGraphicsIter comment
        NewTitleBlocksIter NextTitleBlock delete_DboPageTitleBlocksIter title
    } {
        foreach object [::CaptureRCText::objects $pg $state $new $next $free] {
            set bounds [rectangle [$object GetBoundingBox]]
            if {$kind eq "title"} {
                lassign [::CaptureRCText::point [::CaptureRCText::read $object GetLocation $state]] x y
                lassign $bounds a b c d
                set bounds [list [expr {$a+$x}] [expr {$b+$y}] [expr {$c+$x}] [expr {$d+$y}]]
            }
            if {$bounds eq {0 0 0 0}} {
                ::CaptureRCText::writeRow $out [list REVIEW $number "Unresolved $kind bounding box"]
                continue
            }
            boxRow $out $number $kind $bounds
        }
    }
    return $bindings
}

proc ::CaptureRCFix::snapshot {design path state pagePattern refPattern gridSetting} {
    set out [open $path {WRONLY CREAT EXCL}]
    try {
        fconfigure $out -encoding utf-8 -translation lf
        set views [$design NewViewsIter $state $::IterDefs_SCHEMATICS]
        if {$views eq "NULL" || [$state Failed]} {error "Cannot enumerate design schematics"}
        try {
            set number 0
            set bindings {}
            while {[set view [$views NextView $state]] ne "NULL"} {
                if {[$state Failed]} {error "Cannot read schematic view"}
                set sch [DboViewToDboSchematic $view]
                foreach pg [::CaptureRCText::objects $sch $state NewPagesIter NextPage delete_DboSchematicPagesIter] {
                    if {![string match -nocase $pagePattern [::CaptureRCText::stringField $pg GetName]]} {continue}
                    set bindings [dict merge $bindings [page $sch $pg [incr number] $out $state $refPattern $gridSetting]]
                }
            }
            if {!$number} {error "No matching schematic pages"}
        } finally {
            delete_DboLibViewsIter $views
        }
        return $bindings
    } finally {
        close $out
    }
}

proc ::CaptureRCFix::requireOK {status operation} {
    if {[$status Failed]} {error "$operation failed ([$status Code])"}
}

proc ::CaptureRCFix::setCaption {dp x y rotation state} {
    requireOK [$dp SetRotation $rotation] SetRotation
    requireOK [$dp SetLocation [DboTclHelper_sMakeCPoint $x $y]] SetLocation
    if {[::CaptureRCText::point [::CaptureRCText::read $dp GetLocation $state]] ne [list $x $y] ||
        [::CaptureRCText::read $dp GetRotation $state] != $rotation} {
        error "Capture display property readback differs"
    }
    requireOK [$dp MarkModified] MarkDisplayModified
}

proc ::CaptureRCFix::apply {design plan bindings state} {
    set stream [open $plan r]
    try {
        set changes {}
        while {[gets $stream row] >= 0} {
            if {[llength [split $row \t]] != 5} {error "Invalid layout plan row"}
            lassign [split $row \t] page id x y rotation
            foreach v [list $page $id $x $y $rotation] {
                if {![string is integer -strict $v]} {error "Invalid layout plan"}
            }
            if {$rotation ni {0 3}} {error "Unsupported new display orientation"}
            set key [list $page $id]
            if {[dict exists $changes $key] || ![dict exists $bindings $key]} {
                error "Display property is duplicated or outside the repair scope: $key"
            }
            lassign [dict get $bindings $key] dp part sch pg ox oy oldRotation
            if {[::CaptureRCText::point [::CaptureRCText::read $dp GetLocation $state]] ne [list $ox $oy] ||
                [::CaptureRCText::read $dp GetRotation $state] != $oldRotation} {
                error "Display property changed since the scan: $key"
            }
            dict set changes $key [list $x $y $rotation]
        }
    } finally {
        close $stream
    }
    set history {}
    set changedPages {}
    try {
        dict for {key layout} $changes {
            lassign [dict get $bindings $key] dp part sch pg ox oy oldRotation
            lappend history [list $dp $ox $oy $oldRotation]
            setCaption $dp {*}$layout $state
            requireOK [$part MarkModified] MarkPartModified
            dict lappend changedPages $sch $pg
        }
        dict for {sch pages} $changedPages {
            foreach pg [lsort -unique $pages] {
                requireOK [$pg MarkModified] MarkPageModified
                requireOK [$sch MarkModified $pg] MarkSchematicPageModified
            }
            requireOK [$sch MarkModified] MarkSchematicModified
        }
        if {[dict size $changes]} {$design MarkModified NULL}
    } on error {message options} {
        set rollbackFailed 0
        foreach item [lreverse $history] {
            if {[catch {setCaption {*}$item $state}]} {incr rollbackFailed}
        }
        if {$rollbackFailed} {error "$message; $rollbackFailed rollback entries failed. Close the working copy without saving."}
        return -options $options $message
    }
    return [list [dict size $changes] $changedPages]
}

proc ::CaptureRCFix::save {design changedPages} {
    dict for {sch pages} $changedPages {
        foreach pg [lsort -unique $pages] {requireOK [$sch SavePage $pg] SavePage}
        requireOK [$design SaveSchematic $sch] SaveSchematic
    }
    DboSession -this $::DboSession_s_pDboSession
    requireOK [$::DboSession_s_pDboSession SaveDesign $design] SaveDesign
}

proc ::CaptureRCFix::python {} {
    if {[info exists ::env(CAPTURE_RC_PYTHON)]} {return [list $::env(CAPTURE_RC_PYTHON)]}
    set bundled [file join $::env(USERPROFILE) .cache codex-runtimes codex-primary-runtime dependencies python python.exe]
    if {[file isfile $bundled]} {return [list $bundled]}
    if {[auto_execok py] ne ""} {return [list {*}[auto_execok py] -3]}
    if {[auto_execok python] ne ""} {return [auto_execok python]}
    error "Set CAPTURE_RC_PYTHON to a Python 3 executable"
}

proc ::CaptureRCFix::run {design grid pagePattern refPattern mode output {pageBoundary allow-overflow} {layout avoid}} {
    variable directory
    if {$design eq "NULL" || $design eq ""} {error "Open a design first"}
    set gridSetting [gridSetting $grid]
    if {$mode ni {apply preview apply-save}} {error "Expected apply, preview or apply-save"}
    if {$pageBoundary ni {allow-overflow stay-on-page}} {error "Expected allow-overflow or stay-on-page"}
    set planner [file normalize [file join $directory .. 01AIEDA learning scripts capture_layout_tools plan_capture_rc_text_fix.py]]
    file mkdir $output
    set snapshot [file join $output scene.tsv]
    set plan [file join $output plan.tsv]
    set report [file join $output results.tsv]
    set state [DboState]
    try {
        set bindings [snapshot $design $snapshot $state $pagePattern $refPattern $gridSetting]
        set summary [exec {*}[python] -B $planner $snapshot $plan $report $pageBoundary $layout]
        puts $summary
        if {$mode ne "preview"} {
            lassign [apply $design $plan $bindings $state] changed changedPages
            puts "Applied $changed display-property edits in memory. Inspect the result and save the working copy."
            if {$changed && $mode eq "apply-save"} {
                save $design $changedPages
                puts "Saved the active design."
            }
        }
        puts "Report: $report"
        return $report
    } finally {
        $state -delete
    }
}

proc rc_text_fix {{grid {}} {pagePattern *} {refPattern *} {mode apply} {pageBoundary allow-overflow}} {
    set design [GetActivePMDesign]
    set output [file join $::env(TEMP) "capture_rc_fix_[pid]_[clock clicks]"]
    set report [::CaptureRCFix::run $design $grid $pagePattern $refPattern $mode $output $pageBoundary]
    if {$mode ne "preview"} {catch {ZoomRedraw}}
    return $report
}

proc rc_text_fix_fixed {{grid {}} {pagePattern *} {refPattern *} {mode apply} {pageBoundary allow-overflow}} {
    set design [GetActivePMDesign]
    set output [file join $::env(TEMP) "capture_rc_fixed_[pid]_[clock clicks]"]
    set report [::CaptureRCFix::run $design $grid $pagePattern $refPattern $mode $output $pageBoundary fixed]
    if {$mode ne "preview"} {catch {ZoomRedraw}}
    return $report
}
