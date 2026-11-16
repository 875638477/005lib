"""Read-only COMPACT facts gate. No CAD access, editing, or layout solver.

Proof JSONs bind the same source XML and native manifest. The independent
extractor/reviewer supplies real geometry, full-ink neighborhood and one reasonable
joint candidate budget; this tool measures paths/pitches and checks that budget.
"""
from pathlib import Path
import argparse, collections, hashlib, json, math

VERSION='COMPACT-20261116.1'
CHECKS=('COMPACTGRID','COMPACTFUNCTION','COMPACTCLEAR','COMPACTPITCH','COMPACTLEADS')
class Unknown(ValueError):pass
def need(ok,why):
    if not ok:raise Unknown(why)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def finite(x):return isinstance(x,(int,float)) and not isinstance(x,bool) and math.isfinite(x)
def positive(x):return finite(x) and x>0
def point(p):return isinstance(p,list) and len(p)==2 and all(finite(x) for x in p)
def ordered_ids(rows,key='id'):
    need(isinstance(rows,list),'Missing object inventory')
    ids=[r[key] for r in rows];need(len(ids)==len(set(ids)),'Duplicate object identities');return ids
def resolve(base,p):
    q=Path(p);return q if q.is_absolute() else base/q
def bound(ref,base,source_sha,native_sha=None):
    need(isinstance(ref,dict) and ref.get('path') and ref.get('sha256'),'Missing bound proof')
    p=resolve(base,ref['path']);need(p.is_file() and sha(p)==ref['sha256'],'Missing or stale proof: '+str(p))
    q=json.loads(p.read_text(encoding='utf-8-sig'))
    need(q.get('source_xml_sha256')==source_sha,'Proof/source mismatch: '+str(p))
    if native_sha is not None:need(q.get('native_manifest_sha256')==native_sha,'Proof/native mismatch: '+str(p))
    return q
def path_length(points):
    need(isinstance(points,list) and len(points)>=2 and all(point(p) for p in points),'Incomplete actual path')
    length=0
    for a,b in zip(points,points[1:]):
        need(a!=b and (a[0]==b[0] or a[1]==b[1]),'Path must contain ordered nonzero orthogonal segments')
        length+=math.dist(a,b)
    return length
def pitch(rows,frame):
    u,v,o=frame.get('u'),frame.get('v'),frame.get('origin')
    need(point(u) and point(v) and point(o),'Missing native local group frame')
    need(abs(math.hypot(*u)-1)<1e-8 and abs(math.hypot(*v)-1)<1e-8 and abs(sum(a*b for a,b in zip(u,v)))<1e-8,'Invalid local group frame')
    positions=[]
    for r in rows:
        p=r.get('anchor');need(point(p),'Missing actual row anchor')
        positions.append(sum((p[i]-o[i])*v[i] for i in (0,1)))
    gaps=[b-a for a,b in zip(positions,positions[1:])]
    need(all(x>0 for x in gaps),'Rows are not in functional order')
    return gaps
def ongrid(p,g,o):return all(abs((p[i]-o[i])/g-round((p[i]-o[i])/g))<1e-8 for i in (0,1))
def clear(scene,policy):
    need(isinstance(scene,dict) and scene.get('full_ink') is True and scene.get('neighborhood_complete') is True,'Incomplete full-ink neighborhood')
    need(set(scene.get('covered_roles',[]))==set(policy['required_scene_roles']),'Incomplete scene roles')
    need(scene.get('unresolved_objects')==0 and isinstance(scene.get('interferences'),list),'Unresolved scene objects')
    m,r=scene.get('minimum_clearance_doc'),scene.get('required_clearance_doc')
    need(finite(m) and positive(r),'Missing measured/required clearance')
    return not scene['interferences'] and m+1e-9>=r
def check_group(inv,actual,scene,topo,budget,grid,policy):
    result={k:'UNKNOWN' for k in CHECKS};detail={};gid=inv['id']
    try:
        need(inv.get('functional_role') and inv.get('member_ids'),'Unclassified functional group')
        need(topo.get('reviewed') is True and topo.get('reason') and topo.get('preserved') is True,'Unverified functional topology')
        need(set(topo.get('member_ids',[]))==set(inv['member_ids']),'Topology inventory mismatch')
        need(budget.get('basis_id') and budget.get('reason') and budget.get('constraint_ids') and budget.get('source_kind') in ['approved_native_reference','verified_local_candidate'],'Missing reasonable local budget basis')
        reasons=budget.get('reason_codes',[])
        need(reasons and set(reasons)<=set(policy['allowed_budget_reasons']),'Unsupported budget reason')
        need(budget.get('constraint_ids')==topo.get('constraint_ids'),'Candidate changes functional constraints')
        candidate=budget.get('candidate',{})
        need(candidate.get('independently_verified') is True and candidate.get('electrical_scope_preserved') is True and candidate.get('functional_topology_preserved') is True,'Unverified local candidate')
        result['COMPACTFUNCTION']='PASS'
        g,o=grid.get('grid_doc'),grid.get('origin');ptp=grid.get('pin_to_pin_doc');fraction=grid.get('selected_fraction')
        need(grid.get('units') and positive(g) and positive(ptp) and positive(fraction) and point(o) and abs(g-ptp*fraction)<1e-8,'Unknown current grid/units')
        rows,leads=actual.get('rows',[]),actual.get('leads',[]);cr,cl=candidate.get('rows',[]),candidate.get('leads',[])
        need(ordered_ids(rows)==inv.get('row_ids')==ordered_ids(cr),'Row coverage/order mismatch')
        need(set(ordered_ids(leads))==set(inv.get('lead_ids',[]))==set(ordered_ids(cl)),'Lead coverage mismatch')
        need(rows and leads,'Empty group geometry')
        points=[r.get('anchor') for r in rows+cr]
        for lead in leads+cl:
            need(lead.get('owner_pin_id') and lead.get('terminal_id') and lead.get('net_identity') and lead.get('terminal_kind') in ['offpage','global','part_pin','junction','testpoint'],'Missing actual path identity')
            path_length(lead.get('path'));points+=lead['path']
        need(all(point(p) for p in points),'Unknown geometry')
        result['COMPACTGRID']='PASS' if all(ongrid(p,g,o) for p in points) else 'FAIL'
        for s in [scene,candidate.get('scene',{})]:
            need(set(s.get('covered_member_ids',[]))==set(inv['member_ids']) and set(s.get('covered_lead_ids',[]))==set(inv['lead_ids']),'Incomplete scene object coverage')
        actual_clear=clear(scene,policy);candidate_clear=clear(candidate.get('scene'),policy)
        result['COMPACTCLEAR']='PASS' if actual_clear and candidate_clear else 'FAIL'
        # A colliding candidate can never license compactness or a longer budget.
        need(candidate_clear,'Candidate does not have a valid clear neighborhood')
        slack=budget.get('slack_grids',policy['default_slack_grids'])
        need(isinstance(slack,int) and not isinstance(slack,bool) and 0<=slack<=policy['maximum_context_slack_grids'],'Invalid grid slack')
        if slack>policy['default_slack_grids']:need(budget.get('extra_slack_reason'),'Extra slack needs a local functional reason')
        ap,cp=pitch(rows,actual.get('frame',{})),pitch(cr,actual.get('frame',{}))
        result['COMPACTPITCH']='PASS' if all(a<=b+slack*g+1e-8 for a,b in zip(ap,cp)) else 'FAIL'
        detail['pitch_actual_grids']=[x/g for x in ap];detail['pitch_candidate_grids']=[x/g for x in cp]
        current={x['id']:x for x in leads};target={x['id']:x for x in cl}
        limits={k:path_length(v['path']) for k,v in target.items()}
        shared=budget.get('shared_lead_classes',[]);seen=[]
        for ids in shared:
            need(isinstance(ids,list) and len(ids)>=2 and set(ids)<=set(limits),'Invalid shared short-lead class')
            seen+=ids;length=max(limits[k] for k in ids)
            for k in ids:limits[k]=length
        need(len(seen)==len(set(seen)),'Lead belongs to multiple alignment classes')
        measured={k:path_length(v['path']) for k,v in current.items()}
        for k in current:
            for field in ['owner_pin_id','terminal_id','terminal_kind','net_identity']:
                need(current[k][field]==target[k][field],'Candidate changes lead electrical identity')
        result['COMPACTLEADS']='PASS' if all(measured[k]<=limits[k]+slack*g+1e-8 for k in current) else 'FAIL'
        detail['lead_actual_grids']={k:x/g for k,x in measured.items()};detail['lead_budget_grids']={k:x/g for k,x in limits.items()}
        detail['slack_grids']=slack
    except (Unknown,KeyError,TypeError,ValueError) as e:detail['unknown_reason']=str(e)
    return dict(id=gid,checks=result,details=detail)
def audit(data,base,policy):
    need(data.get('rule_version')==VERSION==policy['version'],'Wrong compactness version')
    need(policy.get('required_checks')==list(CHECKS),'Checker/contract mismatch')
    src=data.get('source_xml',{});p=resolve(base,src.get('path',''))
    need(p.is_file() and src.get('sha256')==sha(p),'Missing or stale XML')
    manifest_ref=data.get('native_manifest');manifest=bound(manifest_ref,base,src['sha256'])
    files=manifest.get('files',[]);need(set(x.get('role') for x in files)>=set(policy['required_native_roles']),'Incomplete native manifest')
    for f in files:
        p=resolve(base,f['path']);need(p.is_file() and sha(p)==f['sha256'],'Stale native file')
    proofs={k:bound(data.get('evidence',{}).get(k),base,src['sha256'],manifest_ref['sha256']) for k in policy['required_evidence']}
    inventory=proofs['inventory'];groups=inventory.get('groups',[]);ids=ordered_ids(groups)
    need(ids and ids==data.get('expected_group_ids') and ids==data.get('group_ids'),'Incomplete independent group coverage')
    members=[k for q in groups for k in q.get('member_ids',[])];expected=inventory.get('expected_rc_ids',[])
    need(expected and len(expected)==len(set(expected)) and len(members)==len(set(members)) and set(members)==set(expected),'Incomplete R/C membership coverage')
    for role in ['geometry','scene','topology','budgets']:need(set(proofs[role].get('groups',{}))==set(ids),'Incomplete '+role+' group evidence')
    rows=[check_group(q,proofs['geometry']['groups'][q['id']],proofs['scene']['groups'][q['id']],proofs['topology']['groups'][q['id']],proofs['budgets']['groups'][q['id']],proofs['geometry'].get('grid',{}),policy) for q in groups]
    states=[s for q in rows for s in q['checks'].values()]
    status='FAIL' if 'FAIL' in states else 'UNKNOWN' if 'UNKNOWN' in states else 'PASS_COMPACT_FACTS'
    return dict(rule_version=VERSION,status=status,coverage_complete=True,groups=rows,scope=policy['checker_scope'])
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--facts',required=True,type=Path);ap.add_argument('--output',required=True,type=Path);a=ap.parse_args()
    policy=json.loads((Path(__file__).resolve().parents[2]/'notes/capture_schematic_compactness_contract.json').read_text(encoding='utf-8'))
    try:result=audit(json.loads(a.facts.read_text(encoding='utf-8-sig')),a.facts.resolve().parent,policy)
    except (Unknown,KeyError,TypeError,ValueError,OSError) as e:result=dict(rule_version=VERSION,status='UNKNOWN',coverage_complete=False,reason=str(e))
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps(result,ensure_ascii=False));return 0 if result['status']=='PASS_COMPACT_FACTS' else 2
if __name__=='__main__':raise SystemExit(main())
