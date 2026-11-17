"""Check same-save normalized R/C facts. Read-only; does not edit CAD or sign off a PCB.

The extractor must bind actual XML, public-library content, native text alignment
and full scene facts. Missing alignment/scene evidence is UNKNOWN, never PASS.
"""
from pathlib import Path
import argparse,hashlib,json,math,collections
VERSION='RC-20261117.1'
CHECKS=('RCLIB','RCAXIS','RCALIGN','RCGRID','RCCLEAR')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def bound_file(evidence,source_sha):
    if not isinstance(evidence,dict) or not evidence.get('path') or not evidence.get('sha256'):return False
    p=Path(evidence['path'])
    if not p.is_file() or sha(p)!=evidence['sha256']:return False
    proof=json.loads(p.read_text(encoding='utf-8-sig'))
    return proof.get('source_xml_sha256')==source_sha
def distance_to_line(p,line):
    a,b=line;dx,dy=b[0]-a[0],b[1]-a[1]
    if not dx and not dy:raise ValueError('Zero alignment reference line')
    return abs(dx*(p[1]-a[1])-dy*(p[0]-a[0]))/math.hypot(dx,dy)
def proximity_result(profile,axis,grid,source_sha):
    """Anchor alignment cannot certify ink-to-lead proximity (2026-11-17)."""
    q=profile.get('proximity',{});detail={}
    if not q.get('native_text_frame_bound') or not q.get('approved_reference_id') or not bound_file(q.get('evidence'),source_sha):
        return 'UNKNOWN',{'proximity_reason':'Missing native text-frame/approved-reference binding'}
    slack=q.get('allowed_quantization_doc')
    if not isinstance(grid,(int,float)) or not grid>0 or not isinstance(slack,(int,float)) or not 0<=slack<=grid:
        return 'UNKNOWN',{'proximity_reason':'Missing current grid or quantization allowance exceeds one grid'}
    checks=[]
    for role in ['ref','value']:
        row=q.get(role,{});bounds=row.get('full_ink_bounds');lead=row.get('own_lead_line');reference=row.get('approved_horizontal_ink_gap_doc')
        if not (isinstance(bounds,list) and len(bounds)==4 and all(isinstance(x,(int,float)) and math.isfinite(x) for x in bounds) and bounds[0]<bounds[2] and bounds[1]<bounds[3] and isinstance(lead,list) and len(lead)==2 and all(isinstance(p,list) and len(p)==2 for p in lead) and lead[0]!=lead[1] and isinstance(reference,(int,float)) and math.isfinite(reference) and reference>=0):
            return 'UNKNOWN',{'proximity_reason':'Missing full glyph bounds, own lead or approved gap: '+role}
        if axis=='horizontal':
            if lead[0][1]!=lead[1][1]:return 'UNKNOWN',{'proximity_reason':'Own lead is not horizontal'}
            y=lead[0][1];gap=y-bounds[3] if bounds[3]<=y else bounds[1]-y if bounds[1]>=y else -1
        elif axis=='vertical':
            if lead[0][0]!=lead[1][0]:return 'UNKNOWN',{'proximity_reason':'Own lead is not vertical'}
            x=lead[0][0];gap=x-bounds[2] if bounds[2]<=x else bounds[0]-x if bounds[0]>=x else -1
        else:return 'UNKNOWN',{'proximity_reason':'Unknown physical component axis'}
        limit=reference+slack;checks.append(0<=gap<=limit+1e-9);detail[role+'_perpendicular_ink_gap_doc']=gap;detail[role+'_maximum_gap_doc']=limit
    return ('PASS' if all(checks) else 'FAIL'),detail
def check_part(p,root,source_sha,policy):
    s={k:'UNKNOWN' for k in CHECKS};why={}
    lib=p.get('library',{});path=lib.get('path');cache=lib.get('cache_shape_sha256');public=lib.get('public_shape_sha256')
    if path and cache and public and lib.get('file_sha256'):
        f=Path(path)
        selected=policy['canonical_symbols'].get(p.get('symbol_type'))
        if selected:
            expected=root/selected['relative_library']
            s['RCLIB']='PASS' if f.is_file() and f.resolve()==expected.resolve() and lib.get('package_name')==selected['package'] and sha(f)==lib['file_sha256'] and cache==public else 'FAIL'
        else:s['RCLIB']='UNKNOWN'
    axis=p.get('component_axis');texts=p.get('texts',{});pair=[texts.get(k,{}) for k in ['ref','value']]
    if axis in ['horizontal','vertical'] and all(t.get('rotation_quadrants') in [0,1,2,3] for t in pair):
        target=0 if axis=='horizontal' else 1
        s['RCAXIS']='PASS' if all(t['rotation_quadrants']%2==target for t in pair) else 'FAIL'
    profile=p.get('alignment',{})
    if profile.get('bound_native_reference') and bound_file(profile.get('evidence'),source_sha) and all(t.get('anchor') is not None and profile.get(k+'_baseline_line') for k,t in zip(['ref','value'],pair)) and profile.get('allowed_native_rounding_doc') is not None:
        tolerance=profile['allowed_native_rounding_doc'];assert 0<=tolerance<1,'Alignment tolerance must be native quantization, not a clearance search radius'
        distances={k:distance_to_line(t['anchor'],profile[k+'_baseline_line']) for k,t in zip(['ref','value'],pair)}
        anchor_ok=all(v<=tolerance+1e-9 for v in distances.values());near,near_detail=proximity_result(profile,axis,p.get('grid_doc'),source_sha)
        s['RCALIGN']='FAIL' if not anchor_ok or near=='FAIL' else 'PASS' if near=='PASS' else 'UNKNOWN';why['alignment_distances_doc']=distances;why.update(near_detail)
    grid=p.get('grid_doc');points=p.get('electrical_grid_points',[])+[t['anchor'] for t in pair if t.get('anchor') is not None]
    if grid and grid>0 and p.get('electrical_grid_coverage_complete') and all(t.get('anchor') is not None for t in pair):
        s['RCGRID']='PASS' if all(abs(v/grid-round(v/grid))<1e-9 for pt in points for v in pt) else 'FAIL'
    scene=p.get('clearance',{})
    if scene.get('bound_full_scene') and bound_file(scene.get('evidence'),source_sha) and set(scene.get('covered_roles',[]))=={'ref','value'} and scene.get('unresolved_objects')==0 and isinstance(scene.get('interferences'),list):
        s['RCCLEAR']='PASS' if not scene['interferences'] else 'FAIL'
    return dict(id=p['id'],page=p.get('page'),ref=p.get('ref'),checks=s,details=why)
def audit(data,root):
    contract=Path(__file__).resolve().parents[2]/'notes/capture_rc_layout_contract.json'
    policy=json.loads(contract.read_text(encoding='utf-8'))
    assert policy['version']==VERSION and policy['required_checks']==list(CHECKS),'Checker and current policy are inconsistent'
    assert data['rule_version']==VERSION,'Wrong rule version'
    source=data['source_xml'];assert sha(source['path'])==source['sha256'],'Stale source XML'
    for x in data.get('evidence_files',[]):assert sha(x['path'])==x['sha256'],('Stale evidence',x['path'])
    ids=data['inventory'];parts=data['parts'];actual=[p['id'] for p in parts]
    coverage=bool(ids) and len(ids)==len(set(ids)) and len(actual)==len(set(actual)) and set(ids)==set(actual)
    rows=[check_part(p,root,source['sha256'],policy) for p in parts];counts={k:dict(collections.Counter(r['checks'][k] for r in rows)) for k in CHECKS}
    ok=coverage and all(s=='PASS' for r in rows for s in r['checks'].values())
    return dict(rule_version=VERSION,rule_contract_sha256=sha(contract),status='PASS_RC_FACTS' if ok else 'NOT_COMPLIANT_OR_UNVERIFIED',coverage_complete=coverage,inventory_count=len(ids),checked_count=len(rows),counts=counts,parts=rows,scope='Bound extracted R/C facts only; native electrical/GUI/PCB validation remain separate. No CAD edits performed.')
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--facts',required=True,type=Path);ap.add_argument('--output',required=True,type=Path);ap.add_argument('--library-root',default='D:/005lib',type=Path);a=ap.parse_args()
    result=audit(json.loads(a.facts.read_text(encoding='utf-8-sig')),a.library_root);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps({k:v for k,v in result.items() if k!='parts'},ensure_ascii=False));return 0 if result['status']=='PASS_RC_FACTS' else 2
if __name__=='__main__':raise SystemExit(main())
