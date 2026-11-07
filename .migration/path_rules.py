import re
from pathlib import Path

PAT=re.compile(rb'D:(\\{1,2}|/)001DIY(?:\\{1,2}|/)005lib(?=[\\/\s\x00"\'\)]|$)',re.I)

TEXT_PAT=re.compile(r'D:(\\{1,2}|/)001DIY(?:\\{1,2}|/)005lib(?=[\\/\s\x00"\'\)]|$)',re.I)

SCRIPT_EXT={'.bat','.cmd','.ps1','.py','.tcl','.il','.ils','.vbs','.js','.scr','.form','.mnu','.menu','.do','.hta','.sh','.pl','.rb'}

CONFIG_EXT={'.ini','.env','.conf','.cfg','.xml','.yaml','.yml','.prf','.prm','.props','.pro','.prj','.opj','.reg','.url','.code-workspace','.pth','.mru','.ilinit','.dbc','.prjpcb','.prjpcbstructure'}

SKIP_PARTS={'.git','node_modules','.pio','__pycache__'}

def transform(data):
    if data.startswith((b'\xff\xfe',b'\xfe\xff')):
        enc='utf-16-le' if data[:2]==b'\xff\xfe' else 'utf-16-be'
        text=data[2:].decode(enc)
        text,n=TEXT_PAT.subn(lambda m:'D:'+m[1]+'005lib',text)
        text,k=re.subn(r'(?i)(D:[\\/]005lib[\\/])01AIcadence([\\/]ai_bridge[\\/]ai_bridge\.il)',lambda m:m[1]+'01AIEDA'+m[2],text)
        return data[:2]+text.encode(enc),n+k
    if b'\0' in data: return data,0
    changed,n=PAT.subn(lambda m:b'D:'+m[1]+b'005lib',data)
    changed,k=re.subn(rb'(D:[\\/]005lib[\\/])01AIcadence([\\/]ai_bridge[\\/]ai_bridge\.il)',lambda m:m[1]+b'01AIEDA'+m[2],changed,flags=re.I)
    return changed,n+k

def current_document(p,data):
    if p.name.lower()=='github_location.txt' and p.parent.name.lower()=='01aieda':
        return ('当前 GitHub 库位置（2026-11-07）\r\nhttps://github.com/875638477/005lib\r\n本机正式库根目录：D:\\005lib\r\n仓库根目录中的 9 个 01* 目录与本机同名目录对应，不再套一层 005lib。\r\n旧的 D:\\001DIY\\005lib 不保留兼容链接；原 001Diy 仓库的库文件在新仓库核验后移除。\r\n历史维护目录中的迁移和删除脚本只作历史证据，请勿直接重跑。\r\n迁移执行记录：D:\\eda库路径整理.md\r\n').encode('utf-8')
    if p.parent.name.lower()=='01aieda' and p.suffix.lower() in {'.md','.txt'}:
        data=transform(data)[0]
        data=re.sub(rb'github\.com/875638477/001Diy/(tree|blob)/(main|master)/005lib',rb'github.com/875638477/005lib/\1/\2',data,flags=re.I)
        data=re.sub(rb'github\.com/875638477/001Diy',b'github.com/875638477/005lib',data,flags=re.I)
        return data
    return data

def reason(p):
    suffix=p.suffix.lower(); name=p.name.lower(); parts={s.lower() for s in p.parts}
    if parts & SKIP_PARTS: return 'excluded dependency or git storage'
    if 'xwechat_files' in parts: return 'received historical attachment; not an installed configuration'
    if p.parent.name.lower()=='01aieda' and suffix in {'.md','.txt'}: return None
    if suffix in SCRIPT_EXT or suffix in CONFIG_EXT or name=='env': return None
    if suffix=='.json':
        if re.search(r'manifest|receipt|result|report|hash|verification|validation|inventory|sync-|local_installation|local_plan|publish_plan|acceptance|review_board',name): return 'historical evidence or hash manifest'
        if name in {'settings.json','launch.json','tasks.json','package.json','tsconfig.json','jsconfig.json','config.json','paths.json','preferences.json'} or re.search(r'(^|[-_])(config|settings|preferences)([-_.]|$)',name): return None
        return 'JSON data or historical evidence; not a runtime configuration'
    return 'historical log, documentation, data or native binary'
