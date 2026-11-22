# OrCAD Capture 17.2. Source in View > Command Window, then run rc_text_check.
# Reads the current design in memory. Writes a fresh UTF-8 TSV report.
namespace eval ::CaptureRCText {
    variable version RC-20261117.1
}

proc ::CaptureRCText::read {obj method state} {
    set result [$obj $method $state]
    if {[$state Failed]} {error "$method failed ([$state Code])"}
    return $result
}

proc ::CaptureRCText::stringField {obj method} {
    set buffer [DboTclHelper_sMakeCString {}]
    try {
        set status [$obj $method $buffer]
        if {[$status Failed]} {error "$method failed ([$status Code])"}
        return [DboTclHelper_sGetConstCharPtr $buffer]
    } finally {
        DboTclHelper_sDeleteCString $buffer
    }
}

proc ::CaptureRCText::point {p} {
    return [list [DboTclHelper_sGetCPointX $p] [DboTclHelper_sGetCPointY $p]]
}

proc ::CaptureRCText::objects {obj state create next destroy} {
    set iter [read $obj $create $state]
    if {$iter eq "NULL"} {error "$create returned NULL"}
    try {
        set items {}
        while {[set item [$iter $next $state]] ne "NULL"} {
            if {[$state Failed]} {error "$next failed ([$state Code])"}
            lappend items $item
        }
        return $items
    } finally {
        $destroy $iter
    }
}

proc ::CaptureRCText::axisForPins {leads} {
    if {[llength $leads] != 2} {return {}}
    lassign $leads first second
    lassign [lindex $first 2] x1 y1
    lassign [lindex $second 2] x2 y2
    if {$x1 == $x2 && $y1 != $y2} {set axis V; set across 0; set along 1
    } elseif {$y1 == $y2 && $x1 != $x2} {set axis H; set across 1; set along 0
    } else {return {}}
    set line [lindex [lindex $first 2] $across]
    foreach lead $leads {
        lassign $lead number start hot
        if {[lindex $start $across] != $line || $start eq $hot} {return {}}
    }
    set a [lindex [lindex $first 2] $along]
    set b [lindex [lindex $second 2] $along]
    set innerA [lindex [lindex $first 1] $along]
    set innerB [lindex [lindex $second 1] $along]
    if {$a > $b} {
        lassign [list $b $a $innerB $innerA] a b innerA innerB
    }
    if {!($a < $innerA && $innerA <= $innerB && $innerB < $b)} {return {}}
    return [list $axis $across $line $innerA $innerB]
}

proc ::CaptureRCText::textFields {part origin state} {
    set fields [dict create ref {} value {}]
    foreach dp [objects $part $state NewDisplayPropsIter NextProp delete_DboDisplayPropsIter] {
        set name [stringField $dp GetName]
        switch -nocase -- $name {
            "Part Reference" - RefDes {set role ref}
            Value - User_Value {set role value}
            default {continue}
        }
        set type [read $dp GetDisplayType $state]
        if {$type == $::DboValue_DO_NOT_DISPLAY} {continue}
        set text [stringField $dp GetValueString]
        set rotation [read $dp GetRotation $state]
        lassign $origin ox oy
        lassign [point [read $dp GetLocation $state]] x y
        set bbox {}
        if {![$dp IsBoundingBoxDirty]} {
            set rectangle [$dp GetBoundingBox]
            lassign [point [DboTclHelper_sGetCRectTopLeft $rectangle]] x1 y1
            lassign [point [DboTclHelper_sGetCRectBottomRight $rectangle]] x2 y2
            if {$x1 < $x2 && $y1 < $y2} {
                set bbox [list [expr {$ox+$x1}] [expr {$oy+$y1}] [expr {$ox+$x2}] [expr {$oy+$y2}]]
            }
        }
        dict lappend fields $role [dict create id [read $dp GetId $state] name $name \
            text $text type $type rotation $rotation anchor [list [expr {$ox+$x}] [expr {$oy+$y}]] bbox $bbox]
    }
    return $fields
}

proc ::CaptureRCText::axisResult {axis entries} {
    if {[llength $entries] != 1} {return [list UNKNOWN MISSING_OR_AMBIGUOUS_DISPLAY]}
    set data [lindex $entries 0]
    if {[dict get $data type] != $::DboValue_VALUE_ONLY || [string trim [dict get $data text]] eq ""} {
        return [list UNKNOWN DISPLAY_TEXT_NEEDS_REVIEW]
    }
    set rotation [dict get $data rotation]
    if {$axis eq "" || $rotation ni {0 1 2 3}} {return [list UNKNOWN AXIS_UNAVAILABLE]}
    if {$rotation % 2 != ($axis eq "V")} {return [list FAIL WRONG_TEXT_AXIS]}
    return [list PASS PARALLEL]
}

proc ::CaptureRCText::writeRow {channel row} {
    set cells {}
    foreach cell $row {
        lappend cells [string map [list \\ \\\\ \t {\t} \n {\n} \r {\r}] $cell]
    }
    puts $channel [join $cells \t]
}

proc ::CaptureRCText::writePart {channel schematic page units grid part id ref symbol library state} {
    set leads {}
    foreach pin [objects $part $state NewPinsIter NextPin delete_DboPartInstPinsIter] {
        lappend leads [list [stringField $pin GetPinNumber] \
            [point [read $pin GetOffsetStartPoint $state]] [point [read $pin GetOffsetHotSpot $state]]]
    }
    lassign [axisForPins $leads] axis across line
    set origin [point [read $part GetLocation $state]]
    set fields [textFields $part $origin $state]
    set counts {}
    foreach role {ref value} {
        set entries [dict get $fields $role]
        lassign [axisResult $axis $entries] status reason
        if {$status eq "PASS"} {
            set other [expr {$role eq "ref" ? "value" : "ref"}]
            set otherEntries [dict get $fields $other]
            if {[llength $otherEntries] == 1 &&
                [dict get [lindex $entries 0] rotation] != [dict get [lindex $otherEntries 0] rotation] &&
                [lindex [axisResult $axis $otherEntries] 0] eq "PASS"} {
                set status FAIL
                set reason OPPOSITE_READING_DIRECTIONS
            }
        }
        dict incr counts $status
        set cells [list $schematic $page $id $ref $symbol $library $axis $units $grid $leads $role]
        set anchorOffset {}
        set display [lrepeat 7 {}]
        if {[llength $entries] == 1} {
            set data [lindex $entries 0]
            set anchor [dict get $data anchor]
            if {$axis ne ""} {set anchorOffset [expr {[lindex $anchor $across] - $line}]}
            set display [list [dict get $data id] [dict get $data name] [dict get $data text] \
                [dict get $data rotation] $anchor $anchorOffset [dict get $data bbox]]
        }
        writeRow $channel [concat $cells $display [list $status $reason UNKNOWN]]
    }
    return $counts
}

proc ::CaptureRCText::audit {design output {grid {}}} {
    variable version
    if {$design eq "" || $design eq "NULL"} {error "Open a schematic design first."}
    if {$grid ne "" && (![string is double -strict $grid] || !($grid > 0 && $grid < Inf))} {
        error "Grid must be a positive number in Capture document units."
    }
    set state [DboState]
    set counts [dict create schematics 0 pages 0 placed 0 candidates 0 PASS 0 FAIL 0 UNKNOWN 0 errors 0]
    set output [file normalize $output]
    set channel [open $output {WRONLY CREAT EXCL}]
    try {
        fconfigure $channel -encoding utf-8 -translation lf
        writeRow $channel [list "# $version" [stringField $design GetName]]
        writeRow $channel {schematic page part_id ref symbol source_library part_axis doc_units_per_inch grid_doc pin_leads role dp_id property text rotation anchor_page anchor_minus_lead_doc native_display_box_page axis_check reason rcalign}
        set views [$design NewViewsIter $state $::IterDefs_SCHEMATICS]
        if {$views eq "NULL" || [$state Failed]} {error "Cannot enumerate design schematics."}
        try {
            while {[set view [$views NextView $state]] ne "NULL"} {
                if {[$state Failed]} {error "Cannot read schematic view ([$state Code])."}
                set schematic [DboViewToDboSchematic $view]
                set sheet [stringField $schematic GetName]
                dict incr counts schematics
                foreach page [objects $schematic $state NewPagesIter NextPage delete_DboSchematicPagesIter] {
                    set pageName [stringField $page GetName]
                    set units [$page GetDocUnitsPerInch]
                    dict incr counts pages
                    foreach object [objects $page $state NewPartInstsIter NextPartInst delete_DboPagePartInstsIter] {
                        set part [DboPartInstToDboPlacedInst $object]
                        if {$part eq "NULL"} {continue}
                        dict incr counts placed
                        set id {}; set ref {}
                        if {[catch {
                            set id [read $part GetId $state]
                            set ref [stringField $part GetReferenceDesignator]
                            set symbol [stringField $part GetSourcePartName]
                            if {![regexp -nocase {(^|[^[:alnum:]])(R|C|PR|PC)([0-9]|\?)} $ref] &&
                                ![regexp -nocase {^(R|C|C_ELEC)(\.|$)} $symbol]} {
                                continue
                            }
                            dict incr counts candidates
                            set lib [stringField $part GetSourceLibName]
                            set tally [writePart $channel $sheet $pageName $units $grid $part $id $ref $symbol $lib $state]
                            dict for {key amount} $tally {dict incr counts $key $amount}
                        } message] == 1} {
                            dict incr counts errors
                            writeRow $channel [concat [list $sheet $pageName $id $ref] \
                                [lrepeat 14 {}] [list UNKNOWN $message UNKNOWN]]
                        }
                    }
                }
            }
        } finally {
            delete_DboLibViewsIter $views
        }
        writeRow $channel [list "# COMPLETE" $counts]
    } finally {
        close $channel
        $state -delete
    }
    puts "RC text ($version): $counts"
    puts "Report: $output"
    return $output
}

proc rc_text_check {{output {}} {grid {}}} {
    if {![info exists ::DboSession_s_pDboSession]} {error "Run in the OrCAD Capture Command Window."}
    set design [GetActivePMDesign]
    if {$output eq ""} {
        set output [file join $::env(TEMP) "capture_rc_text_[pid]_[clock clicks].tsv"]
    }
    return [::CaptureRCText::audit $design $output $grid]
}
