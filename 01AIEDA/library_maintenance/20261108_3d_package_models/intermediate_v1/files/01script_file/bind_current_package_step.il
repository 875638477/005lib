; Run in a PACKAGE symbol editor on an owned copy. Mapping values are in mm.
; args: absolute STEP path, (rx ry rz ox oy oz), absolute output DRA path.
procedure(bindCurrentPackageStep(stepFile poseMM outputDra)
 let((d units factor map m savedName)
  d=axlDBGetDesign() unless(d error("No package drawing is open"))
  unless(isFile(stepFile) error("STEP file does not exist"))
  unless(length(poseMM)==6 error("Expected rx ry rz ox oy oz"))
  unless(axlSetSymbolType("package") error("Package symbol editor is required"))
  units=car(axlDBGetDesignUnits()) factor=if(lowerCase(units)=="mils" then 1.0/0.0254 else 1.0)
  map=list('rotation_x nth(0 poseMM) 'rotation_y nth(1 poseMM) 'rotation_z nth(2 poseMM) 'offset_x nth(3 poseMM)*factor 'offset_y nth(4 poseMM)*factor 'offset_z nth(5 poseMM)*factor)
  unless(axlStepSet('primary nil stepFile map) error("STEP assignment failed"))
  m=axlStepGet(nil 'primary nil) unless(m error("STEP readback failed")) savedName=m->step_name
  unless(axlSaveDesign(?design outputDra ?noMru t ?noConfirm t) error("Save DRA failed"))
  unless(axlOpenDesignForBatch(outputDra "wf") error("Reopen DRA failed"))
  m=axlStepGet(nil 'primary nil) unless(m && m->step_name==savedName error("Mapping did not persist"))
  unless(axlCompileSymbol(?symbol substring(outputDra 1 strlen(outputDra)-4) ?type "PACKAGE" ?silent t ?silentOnOverwrite t) error("Compile PSM failed"))
  printf("STEP saved and reopened; PSM compiled. Model filename: %s\n" savedName) t
 ))
