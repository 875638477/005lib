from native import *
from step_writer import Step
import math
OLD=R/'31_z690i_3d_library_20261108'
rows=json.loads((Q/'model/MODEL_PLAN.json').read_text('utf8'))
by={r['package']:r for r in rows}
black=(.055,.06,.065);gold=(.78,.61,.24);silver=(.62,.65,.67);blue=(.035,.10,.32)
changes=[]
def pins(pkg):
    out=[]
    for line in (OLD/'library_scan/cache'/(pkg+'.tsv')).read_text('utf8').splitlines():
        a=line.split('\t')
        if a[0]=='PIN':out.append(dict(number=a[1],x=float(a[3]),y=float(a[4]),mechanical=a[6]=='t'))
    return out
def begin(pkg):return Step('DETAILED_VISUAL_REFERENCE_'+pkg)
def finish(pkg,s,note):
    r=by[pkg];name=r['model'];p=Q/'models'/name;s.finish(p)
    r.update(rotation=[0,0,0],offset_mm=[0,0,0],kind='detailed_connector_visual_reference',height_status='reference_geometry_not_vendor_release',source_note=note,staged_model=str(p))
    r['height_mm']={'Z690I_PCIEX16':12.,'SODIMM_DDR4_260P_H':6.5,'LP380_G120_B0R01_SOCKET':8.23,'Z690I_ATX_12V_2X4':12.8,'HEADER10X2_2MM_USB3_A':9.,'PIN10_2D54MM_2X05':8.3,'Z690I_USB20':15.6,'SMD-NGFF-75P-M2-H3-LOTES':3.,'SMD-NGFF-75P-M2-H3-LOTES_2242_2230':3.}[pkg]
    changes.append(pkg)
def b(s,x1,y1,z1,x2,y2,z2,c=black):s.box(x1,y1,z1,x2,y2,z2,c)
def terminal(s,p,width=.25,z0=-.05,z1=.25):b(s,p['x']-width/2,p['y']-.55,z0,p['x']+width/2,p['y']+.55,z1,gold)

# This source already points its solder tails down. Its housing bottom is
# z=-21.06 mm, so translate upward; never turn it upside down about X.
r=by['Z690I_DDR4_SOCKET'];r.update(rotation=[0,0,0],offset_mm=[.0178,-.0013,21.06],height_mm=21.06,source_note='Original detailed DDR4 source, solder tails down; housing underside z=-21.06 translated to board surface. Prior X=180 mapping was upside down.')
changes.append(r['package'])

# x16: use actual numbered pad field, NOT the stale PLACE_BOUND which is
# displaced by about 31 mm relative to its own electrical contacts.
pkg='Z690I_PCIEX16';s=begin(pkg);pp=[p for p in pins(pkg)if not p['mechanical']]
xmin=min(p['x']for p in pp)-1.4;xmax=max(p['x']for p in pp)+1.4
yc=(min(p['y']for p in pp)+max(p['y']for p in pp))/2
b(s,xmin,yc-4.4,.05,xmax,yc+4.4,2.)
b(s,xmin,yc-4.4,2.,xmax,yc-.9,10.)
b(s,xmin,yc+.9,2.,xmax,yc+4.4,10.)
for x in [xmin,xmax-.9]:b(s,x,yc-.9,2.,x+.9,yc+.9,10.)
xs=sorted(set(round(p['x'],3)for p in pp));g=max(zip(xs,xs[1:]),key=lambda a:a[1]-a[0]);key=sum(g)/2
b(s,key-.8,yc-.9,2,key+.8,yc+.9,10.)
for p in pp:
    b(s,p['x']-.14,p['y']-.14,-3.,p['x']+.14,p['y']+.14,.25,gold)
    sign=1 if p['y']>yc else -1
    b(s,p['x']-.14,yc+sign*1.05-.12,2.,p['x']+.14,yc+sign*1.05+.12,8.9,gold)
b(s,xmin-4,yc-2,2,xmin,yc+2,12.)
finish(pkg,s,'Detailed visual reference: numbered pad field determines 164 contact positions and key gap. Original PLACE_BOUND is displaced; 2D footprint deliberately unchanged. Nominal 10 mm vertical connector housing, reference latch.')

pkg='SODIMM_DDR4_260P_H';s=begin(pkg);pp=pins(pkg)
b(s,-40.7,-3.2,.05,40.7,3.2,1.)
b(s,-40.7,-3.2,1.,40.7,-.65,5.2)
b(s,-40.7,.65,1.,40.7,3.2,5.2)
elect=[p for p in pp if p['number'].isdigit()and int(p['number'])<=260]
xs=sorted(set(p['x']for p in elect));g=max(zip(xs,xs[1:]),key=lambda a:a[1]-a[0]);key=sum(g)/2
b(s,key-.5,-.65,1.,key+.5,.65,5.2)
for p in elect:
    terminal(s,p,.22)
    yy=.7 if p['y']>0 else -.7
    b(s,p['x']-.10,yy-.08,1.,p['x']+.10,yy+.08,4.65,gold)
for x in [-41.75,41.75]:
    b(s,x-.8,-2.8,0,x+.8,2.8,6.2,silver)
    b(s,x-.6,-1.1,4.3,x+.6,1.1,6.5,black)
finish(pkg,s,'Detailed visual reference matched to current custom 260-pad pattern (0.5 mm pitch, 8.2 mm row spacing, retention pads +/-41.75 mm), keyed slot and end retention. No manufacturer identity assigned; nominal 5.2 mm reference height.')

pkg='LP380_G120_B0R01_SOCKET';s=begin(pkg)
b(s,-27.9,-3.5,.05,27.9,3.5,1.0)
b(s,-27.9,-3.5,1.,27.9,-.8,8.23)
b(s,-27.9,.8,1.,27.9,3.5,8.23)
for x in [-27.9,26.9]:b(s,x,-.8,1.,x+1.,.8,8.23)
b(s,.475,-.8,1.,2.725,.8,8.23)
for p in pins(pkg):
    if not p['mechanical']:
        terminal(s,p,.3)
        yy=.85 if p['y']>0 else -.85
        b(s,p['x']-.13,yy-.07,1.,p['x']+.13,yy+.07,7.65,gold)
    else:s.cylinder(p['x'],p['y'],-1.2,.55,1.3,black,n=16)
finish(pkg,s,'LP380 visual reference: actual 120 SMT pads and locating holes, 8.23 mm library-declared height, keyed opening from native assembly outline. Not a vendor CAD release.')

pkg='Z690I_ATX_12V_2X4';s=begin(pkg);pp=[p for p in pins(pkg)if not p['mechanical']]
# Open 2x4 cavities, keyed corners and all eight real 4.2 mm pitch pins.
for p in pp:
    x,y=p['x'],p['y'];b(s,x-2.1,y-2.1,.05,x+2.1,y+2.1,1.3)
    for a in [(-2.1,-2.1,2.1,-1.55),(-2.1,1.55,2.1,2.1),(-2.1,-1.55,-1.55,1.55),(1.55,-1.55,2.1,1.55)]:
        b(s,x+a[0],y+a[1],1.3,x+a[2],y+a[3],12.8)
    b(s,x-.48,y-.48,-3.2,x+.48,y+.48,8.6,silver)
    if int(p['number'])in [1,2,5,8]:
        for sign in [-1,1]:
            v=[(x+sign*1.55,y+1.55,1.3),(x+sign*.8,y+1.55,1.3),(x+sign*1.55,y+.8,1.3)]
            vv=v+[(a,c,12.8)for a,c,_ in v]
            if sign>0:vv=[vv[i]for i in [1,0,2,4,3,5]]
            s.solid(vv,[(2,1,0),(3,4,5),(0,1,4,3),(1,2,5,4),(2,0,3,5)],black)
b(s,-2.2,-4.1,4.5,2.2,-3.1,11.5)
finish(pkg,s,'Detailed EPS/ATX12V 2x4 visual reference. Each cavity and metal post aligned to actual numbered 4.2 mm pad coordinates; 12.8 mm reference housing height. Key details are illustrative, not vendor release.')

pkg='HEADER10X2_2MM_USB3_A';s=begin(pkg)
b(s,-11.4,-3.6,.05,11.4,3.9,1.5,blue)
for a in [(-11.4,-3.6,-10.2,3.9),(10.2,-3.6,11.4,3.9),(-10.2,-3.6,10.2,-2.5),(-10.2,2.5,-2.,3.9),(2.,2.5,10.2,3.9)]:
    b(s,a[0],a[1],1.5,a[2],a[3],9.,blue)
for p in pins(pkg):
    if not p['mechanical']:b(s,p['x']-.25,p['y']-.25,-2.8,p['x']+.25,p['y']+.25,7.5,gold)
finish(pkg,s,'USB3 internal header detailed visual reference, native 19 pins (missing 20), actual 2 mm positions, open shroud and polarizing wall notch. Nominal 9 mm reference housing height.')

pkg='PIN10_2D54MM_2X05';s=begin(pkg);pp=[p for p in pins(pkg)if not p['mechanical']]
b(s,min(p['x']for p in pp)-1.27,min(p['y']for p in pp)-1.27,.05,max(p['x']for p in pp)+1.27,max(p['y']for p in pp)+1.27,2.5)
for p in pp:b(s,p['x']-.3,p['y']-.3,-2.8,p['x']+.3,p['y']+.3,8.3,gold)
finish(pkg,s,'Front-panel pin header with ten contacts aligned to actual native pads; 2.5 mm insulator / 8.3 mm post reference geometry. Fixes inherited 1 mm placeholder.')

# Rear dual USB-A is a real pair of shell openings, not a header.
pkg='Z690I_USB20';s=begin(pkg)
# Open toward local +X; footprint body x[-4.5,4.24], y[-8.1,8.1].
for z in [1.,9.]:
    for a in [(-4.3,-7.7,-3.8,7.7),(-4.3,-7.7,4.1,-7.3),(-4.3,7.3,4.1,7.7)]:b(s,a[0],a[1],z,a[2],a[3],z+6.6,silver)
    b(s,-4.3,-7.7,z,4.1,7.7,z+.35,silver)
    b(s,-4.3,-7.7,z+6.25,4.1,7.7,z+6.6,silver)
    b(s,-3.8,-5.6,z+1.5,3.4,5.6,z+3.2,black)
    for y in [-3.5,-1.,1.,3.5]:b(s,-2.8,y-.3,z+3.2,2.6,y+.3,z+3.35,gold)
for p in pins(pkg):
    if not p['mechanical']:
        num=int(p['number']);w=.3 if num<=8 else .5
        b(s,p['x']-w,p['y']-w,-2.8,p['x']+w,p['y']+w,.2,silver)
finish(pkg,s,'Dual stacked USB-A receptacle visual reference with two open metal shells, tongues and 8 signal / 4 shell pins aligned to native footprint. Opening toward local +X; dimensions reference only.')

for pkg in ['SMD-NGFF-75P-M2-H3-LOTES','SMD-NGFF-75P-M2-H3-LOTES_2242_2230']:
    s=begin(pkg);pp=pins(pkg)
    b(s,-10.8,-2.9,.05,10.8,3.1,.9)
    b(s,-10.8,1.65,.9,10.8,3.1,3.)
    b(s,-10.8,-2.9,2.,10.8,1.65,3.)
    for x in [-10.8,9.9]:b(s,x,-2.9,.9,x+.9,1.65,2.)
    elect=[p for p in pp if p['number'].isdigit()]
    xs=sorted(set(p['x']for p in elect));g=max(zip(xs,xs[1:]),key=lambda a:a[1]-a[0]);key=sum(g)/2
    b(s,key-.55,-2.9,.9,key+.55,1.65,2.)
    for p in pp:
        if not p['mechanical']:
            terminal(s,p,.2 if p['number'].isdigit()else .6)
            if p['number'].isdigit():b(s,p['x']-.07,-2.2,1.1,p['x']+.07,.7,1.25,gold)
        elif abs(p['y'])<5:s.cylinder(p['x'],p['y'],-.9,.55,1.,black,n=12)
    finish(pkg,s,'M.2 Key M 3 mm detailed visual reference derived from actual pin field and key gap; open card mouth toward negative local Y, retention holes excluded from connector body. No SSD or mounting-hole cover blocks.')

write(Q/'model/MODEL_PLAN.json',rows)
write(Q/'model/CHANGED_PACKAGES.json',changes)
write(Q/'model/MODEL_PROVENANCE.json',[{k:r.get(k)for k in ['package','model','kind','rotation','offset_mm','height_status','source_note']}for r in rows if r['package']in changes])
print('CORRECTED_PACKAGES',len(changes),'DETAILED_STEP_FILES',len(list((Q/'models').glob('*.step'))))
