"""Write closed polyhedral solids as portable AP214 STEP, in millimeters."""
from pathlib import Path
import math
def real(v):
    s=f'{float(v):.9g}'.upper()
    if 'E' in s:
        a,b=s.split('E');return (a if '.' in a else a+'.')+'E'+b
    return s if '.' in s else s+'.'
class Step:
    def __init__(self,title):self.rows=[];self.title=title;self.solids=[];self.styles=[]
    def entity(self,text):
        self.rows.append(text);return '#'+str(len(self.rows))
    def solid(self,vertices,faces,color=(.13,.14,.16)):
        ids=[self.entity("CARTESIAN_POINT('',(%s))"%','.join(real(v) for v in p))for p in vertices]
        vertices_ids=[self.entity("VERTEX_POINT('',%s)"%p)for p in ids]
        edges={}
        fids=[]
        for face in faces:
            oriented=[]
            for i,a in enumerate(face):
                b=face[(i+1)%len(face)];key=tuple(sorted([a,b]))
                if key not in edges:
                    lo,hi=key;delta=[vertices[hi][j]-vertices[lo][j]for j in range(3)]
                    length=math.sqrt(sum(v*v for v in delta))
                    direction=self.entity("DIRECTION('',(%s))"%','.join(real(v/length)for v in delta))
                    vector=self.entity("VECTOR('',%s,%s)"%(direction,real(length)))
                    line=self.entity("LINE('',%s,%s)"%(ids[lo],vector))
                    edges[key]=self.entity("EDGE_CURVE('',%s,%s,%s,.T.)"%(vertices_ids[lo],vertices_ids[hi],line))
                oriented.append(self.entity("ORIENTED_EDGE('',*,*,%s,%s)"%(edges[key],'.T.'if a==key[0]else'.F.')))
            loop=self.entity("EDGE_LOOP('',(%s))"%','.join(oriented))
            bound=self.entity("FACE_OUTER_BOUND('',%s,.T.)"%loop)
            p,q,r=[vertices[i]for i in face[:3]]
            u=[q[i]-p[i]for i in range(3)];v=[r[i]-p[i]for i in range(3)]
            n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
            un=math.sqrt(sum(x*x for x in u));nn=math.sqrt(sum(x*x for x in n))
            if nn<1e-14:continue
            normal=self.entity("DIRECTION('',(%s))"%','.join(real(x/nn)for x in n))
            tangent=self.entity("DIRECTION('',(%s))"%','.join(real(x/un)for x in u))
            placement=self.entity("AXIS2_PLACEMENT_3D('',%s,%s,%s)"%(ids[face[0]],normal,tangent))
            plane=self.entity("PLANE('',%s)"%placement)
            fids.append(self.entity("ADVANCED_FACE('',(%s),%s,.T.)"%(bound,plane)))
        shell=self.entity("CLOSED_SHELL('',(%s))"%','.join(fids))
        brep=self.entity("MANIFOLD_SOLID_BREP('',%s)"%shell);self.solids.append(brep)
        rgb=self.entity("COLOUR_RGB('',%s)"%','.join(map(real,color)))
        fc=self.entity("FILL_AREA_STYLE_COLOUR('',%s)"%rgb)
        fill=self.entity("FILL_AREA_STYLE('',(%s))"%fc)
        sf=self.entity("SURFACE_STYLE_FILL_AREA(%s)"%fill)
        side=self.entity("SURFACE_SIDE_STYLE('',(%s))"%sf)
        usage=self.entity("SURFACE_STYLE_USAGE(.BOTH.,%s)"%side)
        pa=self.entity("PRESENTATION_STYLE_ASSIGNMENT((%s))"%usage)
        self.styles.append(self.entity("STYLED_ITEM('',(%s),%s)"%(pa,brep)))
    def box(self,x1,y1,z1,x2,y2,z2,color=(.13,.14,.16)):
        if min(x2-x1,y2-y1,z2-z1)<=0:return
        self.solid([(x1,y1,z1),(x2,y1,z1),(x2,y2,z1),(x1,y2,z1),(x1,y1,z2),(x2,y1,z2),(x2,y2,z2),(x1,y2,z2)],[(3,2,1,0),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7),(4,5,6,7)],color)
    def cylinder(self,cx,cy,z,r,h,color=(.25,.25,.27),n=48):
        v=[(cx+r*math.cos(i*2*math.pi/n),cy+r*math.sin(i*2*math.pi/n),z+dz)for dz in [0,h]for i in range(n)]
        faces=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n)for i in range(n)]
        self.solid(v,faces,color)
    def finish(self,path):
        app=self.entity("APPLICATION_CONTEXT('automotive_design')")
        self.entity("APPLICATION_PROTOCOL_DEFINITION('international standard','automotive_design',2000,%s)"%app)
        pc=self.entity("PRODUCT_CONTEXT('',%s,'mechanical')"%app)
        p=self.entity("PRODUCT('%s','%s','reference geometry',(%s))"%(self.title,self.title,pc))
        pdf=self.entity("PRODUCT_DEFINITION_FORMATION('','',%s)"%p)
        pdc=self.entity("PRODUCT_DEFINITION_CONTEXT('part definition',%s,'design')"%app)
        pd=self.entity("PRODUCT_DEFINITION('','',%s,%s)"%(pdf,pdc))
        pds=self.entity("PRODUCT_DEFINITION_SHAPE('','',%s)"%pd)
        mm=self.entity("(LENGTH_UNIT() NAMED_UNIT(*) SI_UNIT(.MILLI.,.METRE.))")
        angle=self.entity("(NAMED_UNIT(*) PLANE_ANGLE_UNIT() SI_UNIT($,.RADIAN.))")
        solidangle=self.entity("(NAMED_UNIT(*) SI_UNIT($,.STERADIAN.) SOLID_ANGLE_UNIT())")
        uncertainty=self.entity("UNCERTAINTY_MEASURE_WITH_UNIT(LENGTH_MEASURE(1.E-7),%s,'distance_accuracy_value','')"%mm)
        ctx=self.entity("(GEOMETRIC_REPRESENTATION_CONTEXT(3) GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT((%s)) GLOBAL_UNIT_ASSIGNED_CONTEXT((%s,%s,%s)) REPRESENTATION_CONTEXT('',''))"%(uncertainty,mm,angle,solidangle))
        rep=self.entity("ADVANCED_BREP_SHAPE_REPRESENTATION('',(%s),%s)"%(','.join(self.solids),ctx))
        self.entity("SHAPE_DEFINITION_REPRESENTATION(%s,%s)"%(pds,rep))
        if self.styles:self.entity("MECHANICAL_DESIGN_GEOMETRIC_PRESENTATION_REPRESENTATION('',(%s),%s)"%(','.join(self.styles),ctx))
        out="ISO-10303-21;\nHEADER;\nFILE_DESCRIPTION(('Package reference geometry; see companion provenance manifest'),'2;1');\nFILE_NAME('%s','2026-11-08T00:00:00',(''),(''),'LGA1700 library tools','','');\nFILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\nENDSEC;\nDATA;\n"%Path(path).name
        out+='\n'.join('#%d=%s;'%(i+1,x)for i,x in enumerate(self.rows))+'\nENDSEC;\nEND-ISO-10303-21;\n'
        Path(path).parent.mkdir(parents=True,exist_ok=True);Path(path).write_text(out,'ascii')
