import os,json,subprocess,hashlib,tempfile
from pathlib import Path
from path_rules import transform,reason,current_document
CHECKOUT=Path(os.environ['GITHUB_WORKSPACE']);META=CHECKOUT/'.migration'
EVIDENCE=Path(os.environ['RUNNER_TEMP'])/'migration-evidence';EVIDENCE.mkdir(exist_ok=True)
SOURCE=Path(os.environ['RUNNER_TEMP'])/'source-library'
expected=json.loads((META/'expected.json').read_text())
def run(*args,input=None,cwd=SOURCE):
    r=subprocess.run(list(args),cwd=cwd,input=input,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if r.returncode:raise RuntimeError(r.stderr.decode('utf-8','replace')[-6000:])
    return r.stdout
def git(*args,**kw):return run('git',*args,**kw)
def tree(rev):
    result={}
    for row in git('ls-tree','-r','-z',rev).split(b'\0'):
        if not row:continue
        meta,path=row.split(b'\t',1);mode,kind,oid=meta.split()
        result[path.decode('utf-8')]=dict(mode=mode.decode(),kind=kind.decode(),blob=oid.decode())
    return result
run('git','clone','--no-checkout','https://github.com/875638477/001Diy.git',str(SOURCE),cwd=CHECKOUT)
source=git('rev-parse','HEAD').decode().strip();assert source==expected['source_commit'],'Source changed; refusing a stale migration'
source_tree=git('rev-parse','HEAD:005lib').decode().strip();assert source_tree==expected['source_subtree']
(SOURCE/'005lib').mkdir(exist_ok=True)
split=git('subtree','split','--prefix=005lib','-b','library-split',source).decode().strip().splitlines()[-1]
assert git('rev-parse',split+'^{tree}').decode().strip()==source_tree
git('config','user.name','github-actions[bot]');git('config','user.email','41898282+github-actions[bot]@users.noreply.github.com')
before=tree(split);changes=[];payloads=[]
shortcuts={e['path']:e for e in expected['shortcuts']}
batch=subprocess.Popen(['git','cat-file','--batch'],cwd=SOURCE,stdin=subprocess.PIPE,stdout=subprocess.PIPE)
for path,entry in before.items():
    if reason(Path(path)) is not None and entry['mode']!='120000' and path not in shortcuts:continue
    batch.stdin.write((entry['blob']+'\n').encode());batch.stdin.flush()
    header=batch.stdout.readline().split();assert len(header)==3 and header[1]==b'blob'
    data=batch.stdout.read(int(header[2]));assert batch.stdout.read(1)==b'\n'
    desired,n=transform(data);desired=current_document(Path(path),desired)
    if path in shortcuts:
        shortcut=shortcuts[path];assert hashlib.sha256(data).hexdigest()==shortcut['before_sha256']
        desired=(META/shortcut['payload']).read_bytes();assert hashlib.sha256(desired).hexdigest()==shortcut['after_sha256'];n=1
    if data!=desired:
        payloads.append((path,entry['mode'],desired))
        changes.append(dict(path=path,mode=entry['mode'],before_blob=entry['blob'],before_sha256=hashlib.sha256(data).hexdigest(),after_sha256=hashlib.sha256(desired).hexdigest(),replacements=n))
batch.stdin.close();assert batch.wait()==0
payloads.append(('README.md','100644',(META/'new-readme.md').read_bytes()))
identity=git('var','GIT_COMMITTER_IDENT').strip();stream=bytearray()
for i,(path,mode,data) in enumerate(payloads,1):stream.extend(b'blob\nmark :'+str(i).encode()+b'\ndata '+str(len(data)).encode()+b'\n'+data+b'\n')
message=b'Relocate the EDA library root and update operational paths\n'
stream.extend(b'commit refs/heads/library-patched\ncommitter '+identity+b'\ndata '+str(len(message)).encode()+b'\n'+message+b'from '+split.encode()+b'\n')
for i,(path,mode,data) in enumerate(payloads,1):stream.extend(b'M '+mode.encode()+b' :'+str(i).encode()+b' '+json.dumps(path,ensure_ascii=False).encode()+b'\n')
stream.extend(b'\n');git('fast-import','--quiet',input=bytes(stream))
patched=git('rev-parse','library-patched').decode().strip();after=tree(patched)
assert set(after)-set(before)=={'README.md'} and not(set(before)-set(after))
changed_paths={e['path'] for e in changes}
for path,entry in before.items():
    assert after[path]['mode']==entry['mode']
    if path not in changed_paths:assert after[path]==entry
for entry in changes:
    entry['after_blob']=after[entry['path']]['blob']
    data=git('cat-file','blob',entry['after_blob']);assert hashlib.sha256(data).hexdigest()==entry['after_sha256']
git('fetch','--no-tags',str(CHECKOUT),'HEAD:refs/heads/migration-bootstrap')
bootstrap=git('rev-parse','migration-bootstrap').decode().strip();assert bootstrap==os.environ['GITHUB_SHA']
final_tree=git('rev-parse','library-patched^{tree}').decode().strip()
final=git('commit-tree',final_tree,'-p',bootstrap,'-p',patched,input=b'Migrate verified 005lib library and retain its original history\n').decode().strip()
git('update-ref','refs/heads/library-result',final)
git('fsck','--full','--no-reflogs')
receipt=dict(source_commit=source,source_subtree=source_tree,split_commit=split,original_library_commits=int(git('rev-list','--count',split)),bootstrap_commit=bootstrap,patched_commit=patched,final_commit=final,final_tree=final_tree,files_before=len(before),files_after=len(after),changed_files=changes,unchanged_files=len(before)-len(changes),added_files=['README.md'],removed_files=[],run_id=os.environ['GITHUB_RUN_ID'],verified_before_push=True)
(EVIDENCE/'receipt.json').write_text(json.dumps(receipt,ensure_ascii=False,indent=2),encoding='utf-8')
(EVIDENCE/'before-manifest.json').write_text(json.dumps(before,ensure_ascii=False,indent=2),encoding='utf-8')
(EVIDENCE/'after-manifest.json').write_text(json.dumps(after,ensure_ascii=False,indent=2),encoding='utf-8')
git('fetch',str(SOURCE),'library-result:refs/heads/migration-result',cwd=CHECKOUT)
git('push','origin','refs/heads/migration-result:refs/heads/main',cwd=CHECKOUT)
remote=git('ls-remote','origin','refs/heads/main',cwd=CHECKOUT).decode().split()[0];assert remote==final
receipt['remote_head_verified']=remote
(EVIDENCE/'receipt.json').write_text(json.dumps(receipt,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in receipt.items() if k!='changed_files'},ensure_ascii=False,indent=2))
