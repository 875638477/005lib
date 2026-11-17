"""Strict subset reader of the installed PScript5 bitmap-text output.
Extracts actual printed glyph ink (not a guessed Capture anchor transform).
Unsupported records/missing or ambiguous matches remain UNKNOWN.
"""
from pathlib import Path
import re

NUM=r'-?\d+(?:\.\d+)?'
GLYPH=r'\b(\d+)\s*/g(\d+)\s*\[([^\]]+)\]\s*/g\d+\s*\[(\d+\s+\d+\s+true\s+\[[^\]]+\]\s+0\s+0)\]\s*\[<([0-9a-fA-F\s]+)>\s*\]\s*/(TT\w+) AddT3T32Char'
PSSTRING=r'(?:<[0-9a-fA-F]+>|\((?:\\(?:[0-7]{1,3}|.)|[^\\()])*\))'
TOKEN=re.compile(
 r'(?P<page>%%Page: (\d+) (\d+))|'
 r'(?P<base>/(TT\w+)\s*\[('+NUM+r') 0 0 0 0 0\s*\].*?GreNewFont)|'
 r'(?P<glyph>'+GLYPH+r')|'
 r'(?P<map>F /([F]\d+) 0 /0 F /(TT\w+) mF)|'
 r'(?P<wrap>/([F]\w+) ([F]\d+) \[([^\]]+)\] mFS)|'
 r'(?P<select>\b([F]\w+) Ji)|'
 r'(?P<show>('+NUM+r') ('+NUM+r') M ('+PSSTRING+r')S)',re.S)

def string_bytes(s):
 if s.startswith('<'):return bytes.fromhex(s[1:-1])
 out=bytearray();i=1
 while i<len(s)-1:
  if s[i]!='\\':out.append(ord(s[i]));i+=1;continue
  i+=1;m=re.match(r'[0-7]{1,3}',s[i:])
  if m:out.append(int(m[0],8));i+=len(m[0])
  else:out.append({'n':10,'r':13,'t':9,'b':8,'f':12}.get(s[i],ord(s[i])));i+=1
 return bytes(out)

def parse(ps):
 # PostScript is a byte language. Latin-1 preserves CID bytes and appended
 # platform-encoded annotation bytes; source page identities are checked below.
 text=Path(ps).read_bytes().decode('latin1');fonts={};maps={};wraps={};selected=None;page=0;out=[];unknown=[]
 for m in TOKEN.finditer(text):
  if m.group('page'):page=int(re.search(r'%%Page: (\d+)',m.group()).group(1))
  elif m.group('base'):
   name,n=re.search(r'/(TT\w+)\s*\[('+NUM+')',m.group()).groups();fonts[name]=dict(n=float(n),glyphs={})
  elif m.group('glyph'):
   z=re.fullmatch(GLYPH,m.group(),re.S)
   code,gid,met,bmp,data,font=z.groups();vals=list(map(float,met.split()));iv=bmp.split();w,h=int(iv[0]),int(iv[1]);raw=bytes.fromhex(data);stride=(w+7)//8;assert len(raw)==stride*h,(font,code,w,h,len(raw))
   ink=[(x,y) for y in range(h) for x in range(w) if raw[y*stride+x//8]&(128>>(x%8))]
   mx=list(map(float,re.search(r'\[([^\]]+)\]',bmp).group(1).split()));assert mx[:4]==[1,0,0,1]
   # ImageMatrix maps user coords -> bitmap coords; invert its translation.
   ox,oy=-mx[4],-mx[5];bb=[min(x for x,y in ink)+ox,min(y for x,y in ink)+oy,max(x for x,y in ink)+1+ox,max(y for x,y in ink)+1+oy] if ink else [0,0,0,0]
   fonts[font]['glyphs'][int(code)]=dict(gid=int(gid),advance=vals[0],bbox=bb,bitmap=dict(w=w,h=h,data=raw.hex(),origin=[ox,oy]))
  elif m.group('map'):
   alias,font=re.search(r'F /(F\d+) 0 /0 F /(TT\w+) mF',m.group()).groups();maps[alias]=font
  elif m.group('wrap'):
   name,alias,matrix=re.search(r'/(F\w+) (F\d+) \[([^\]]+)\] mFS',m.group()).groups();mat=list(map(float,matrix.split()));font=maps[alias]
   if font not in fonts or mat[1:3]!=[0,0] or mat[4:]!=[0,0] or abs(mat[0]+mat[3])>=1e-4:wraps[name]=(None,None);continue
   n=fonts[font]['n'];wraps[name]=(font,mat[0]/n)
  elif m.group('select'):selected=re.search(r'\b(F\w+) Ji',m.group()).group(1)
  elif m.group('show'):
   if selected not in wraps:unknown.append(dict(page=page,record=m.group()[:100],reason='unmapped font'));continue
   x,y,seq=re.search('('+NUM+') ('+NUM+') M ('+PSSTRING+')S',m.group()).groups();x,y=float(x),float(y);font,s=wraps[selected]
   if font is None:unknown.append(dict(page=page,record=m.group()[:100],reason='unsupported native font/matrix'));continue
   for cid in string_bytes(seq):
    g=fonts[font]['glyphs'].get(cid)
    if g is None:unknown.append(dict(page=page,record=m.group(),reason='missing glyph'));break
    bb=[x+g['bbox'][0]*s,y+g['bbox'][1]*s,x+g['bbox'][2]*s,y+g['bbox'][3]*s];out.append(dict(page=page,font=font,gid=g['gid'],bbox=bb,baseline=[x,y],scale=s,bitmap=g['bitmap']));x+=g['advance']*s
 return text,out,unknown


if __name__ == '__main__':
 import argparse,json,hashlib
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--ps',required=True,type=Path);ap.add_argument('--output',required=True,type=Path);a=ap.parse_args()
 ps,glyphs,unknown=parse(a.ps)
 result=dict(status='SUPPORTED_BITMAP_RECORDS_EXTRACTED_NOT_DESIGN_SIGNOFF',source_postscript=str(a.ps.resolve()),source_sha256=hashlib.sha256(a.ps.read_bytes()).hexdigest(),glyphs=glyphs,unsupported_records=unknown,scope='Raw native print coordinates and embedded bitmap ink only. Bind page/owner/font/glyph indices to a same-save CAD source and validate physical coordinates independently. Unsupported outline fonts/matrices remain unknown; no claimed page completeness.')
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
 print(json.dumps(dict(glyphs=len(glyphs),unsupported_records=len(unknown),status=result['status'])))
