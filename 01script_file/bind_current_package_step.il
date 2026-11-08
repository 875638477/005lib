; S045 board workflow. Use only a PCB copy, save/reopen, then export DRA/PSM
; with dump_libraries -a -c -x and verify a STEP-only refresh on another copy.
; stepFile is absolute; poseMM = (rx ry rz ox oy oz).
procedure(bindBoardPackageStep(refdes stepFile poseMM)
 let((d c def map factor m)
  d=axlDBGetDesign() unless(d error("No PCB is open"))
  c=axlDBFindByName('refdes refdes)
  unless(c && c->symbol && c->symbol->definition error("Refdes has no placed package"))
  def=c->symbol->definition
  unless(isFile(stepFile) error("Actual STEP file is required"))
  unless(length(poseMM)==6 error("Expected rx ry rz ox oy oz"))
  ; This S045 setter consumes internal mil offsets; axlStepGet reports current
  ; drawing units. A saved/reopened board and exported-library reload are required.
  factor=1.0/0.0254
  map=list('rotation_x nth(0 poseMM) 'rotation_y nth(1 poseMM) 'rotation_z nth(2 poseMM) 'offset_x nth(3 poseMM)*factor 'offset_y nth(4 poseMM)*factor 'offset_z nth(5 poseMM)*factor)
  unless(axlStepSet('primary def stepFile map) error("Mapping failed"))
  m=axlStepGet(nil 'primary def) unless(m error("Mapping readback failed"))
  printf("Mapped %s to %s. Save/reopen and verify before library export.\n" refdes m->step_name) t
 ))
