#!/usr/bin/env python3
import argparse, hashlib, struct, sys
from dataclasses import dataclass

XEX_MAGIC=b'XEX2'
XEX_MODULE_TITLE=0x00000001
XEX_IMAGE_PAGE_SIZE_4KB=0x10000000
XEX_IMAGE_REGION_FREE=0x20000000
XEX_REGION_ALL=0xFFFFFFFF
XEX_MEDIA_ALL=0xFFFFFFFF
XEX_HEADER_FILE_FORMAT_INFO=0x000003FF
XEX_HEADER_ENTRY_POINT=0x00010100
XEX_HEADER_IMPORT_LIBRARIES=0x000103FF
XEX_HEADER_TLS_INFO=0x00020104
XEX_HEADER_SYSTEM_FLAGS=0x00030000
XEX_SYSTEM_GAMEPAD_DISCONNECT=0x20
XEX_SYSTEM_INSECURE_SOCKETS=0x40
XEX_SYSTEM_XAM_HOOKS=0x1000
XEX_SYSTEM_ALLOW_BACKGROUND_DOWNLOAD=0x80000
XEX_SYSTEM_ALLOW_CONTROLLER_SWAPPING=0x40000000
PE_MACHINE_POWERPCBE=0x01F2
PE_SUBSYSTEM_XBOX=0x000E
PE_ORDINAL_FLAG=0x80000000

KNOWN_IMPORT_IDS={
    'xboxkrnl.exe':0x45DC17E0,
    'xam.xex':0xFCA15C76,
    'xbdm.xex':0xECEB8109,
}

def u16le(b,o): return struct.unpack_from('<H',b,o)[0]
def u32le(b,o): return struct.unpack_from('<I',b,o)[0]
def be16(v): return struct.pack('>H',v)
def be32(v): return struct.pack('>I',v)
def align(v,a): return (v+a-1)&~(a-1)

@dataclass
class Section:
    name:str; vsize:int; rva:int; raw_size:int; raw_off:int; chars:int

@dataclass
class ImportLib:
    name:str; iat_rva:int; ordinals:list; addresses:list

class PEError(Exception): pass

class XboxPE:
    def __init__(self,data):
        self.data=data
        if len(data)<0x100 or data[:2]!=b'MZ': raise PEError('not a PE/MZ file')
        self.peoff=u32le(data,0x3c)
        if self.peoff+24>len(data) or data[self.peoff:self.peoff+4]!=b'PE\0\0': raise PEError('PE signature missing')
        self.machine=u16le(data,self.peoff+4); self.nsec=u16le(data,self.peoff+6)
        self.opt_size=u16le(data,self.peoff+20); self.opt=self.peoff+24
        if self.machine!=PE_MACHINE_POWERPCBE: raise PEError('wrong machine 0x%04X; expected POWERPCBE 0x01F2'%self.machine)
        if self.opt+self.opt_size>len(data): raise PEError('truncated optional header')
        self.entry=u32le(data,self.opt+0x10); self.image_base=u32le(data,self.opt+0x1c)
        self.section_align=u32le(data,self.opt+0x20); self.size_headers=u32le(data,self.opt+0x3c)
        self.subsystem=u16le(data,self.opt+0x44)
        if self.subsystem!=PE_SUBSYSTEM_XBOX: raise PEError('wrong subsystem 0x%04X; expected Xbox 0x000E'%self.subsystem)
        if self.section_align not in (0x1000,0x10000): raise PEError('section alignment must be 0x1000 or 0x10000')
        st=self.opt+self.opt_size; self.sections=[]
        for i in range(self.nsec):
            o=st+i*40
            if o+40>len(data): raise PEError('truncated section table')
            name=data[o:o+8].split(b'\0',1)[0].decode('ascii','replace')
            s=Section(name,u32le(data,o+8),u32le(data,o+12),u32le(data,o+16),u32le(data,o+20),u32le(data,o+36))
            if s.raw_size and s.raw_off+s.raw_size>len(data): raise PEError('section %s exceeds file'%name)
            self.sections.append(s)
        self.import_rva=u32le(data,self.opt+0x68) if self.opt_size>=0x70 else 0

    def rva_to_file(self,rva):
        if rva<self.size_headers: return rva
        for s in self.sections:
            if s.rva<=rva<s.rva+max(s.vsize,s.raw_size):
                d=rva-s.rva
                return None if d>=s.raw_size else s.raw_off+d
        return None

    def cstr_at_rva(self,rva):
        o=self.rva_to_file(rva)
        if o is None: raise PEError('invalid string RVA 0x%X'%rva)
        e=self.data.find(b'\0',o)
        if e<0: raise PEError('unterminated import name')
        return self.data[o:e].decode('ascii','replace')

    def imports(self):
        if not self.import_rva: return []
        base=self.rva_to_file(self.import_rva)
        if base is None: raise PEError('invalid import directory RVA')
        out=[]; idx=0
        while True:
            o=base+idx*20
            if o+20>len(self.data): raise PEError('truncated import directory')
            oft,ts,fc,name_rva,iat_rva=struct.unpack_from('<IIIII',self.data,o)
            if not any((oft,ts,fc,name_rva,iat_rva)): break
            name=self.cstr_at_rva(name_rva)
            to=self.rva_to_file(iat_rva)
            if to is None: raise PEError('invalid IAT RVA for '+name)
            ords=[]; n=0
            while True:
                if to+n*4+4>len(self.data): raise PEError('truncated IAT')
                v=u32le(self.data,to+n*4)
                if v==0: break
                if v&PE_ORDINAL_FLAG: ordinal=v&0xffff
                else:
                    no=self.rva_to_file(v)
                    if no is None or no+2>len(self.data): raise PEError('invalid hint/name RVA')
                    ordinal=u16le(self.data,no)
                ords.append(ordinal); n+=1
                if n>65535: raise PEError('too many imports')
            out.append(ImportLib(name,iat_rva,ords,[])); idx+=1
            if idx>255: raise PEError('too many import libraries')
        return out

    def mapped(self,imports):
        end=max([self.size_headers]+[s.rva+s.raw_size for s in self.sections])
        image=bytearray(align(end,self.section_align))
        image[:min(self.size_headers,len(self.data))]=self.data[:min(self.size_headers,len(self.data))]
        for s in self.sections:
            if s.raw_size: image[s.rva:s.rva+s.raw_size]=self.data[s.raw_off:s.raw_off+s.raw_size]
        for mi,lib in enumerate(imports):
            lib.addresses=[]
            for j,ordinal in enumerate(lib.ordinals):
                rva=lib.iat_rva+j*4
                if rva+4>len(image): raise PEError('IAT exceeds mapped image')
                image[rva:rva+4]=be32((ordinal&0xffff)|((mi&0xff)<<16))
                lib.addresses.append(self.image_base+rva)
        return image

    def section_type_for_page(self,offset):
        sec=None
        for s in sorted(self.sections,key=lambda x:x.rva):
            if offset>=s.rva: sec=s
            else: break
        if sec is None: return 3
        if sec.chars&0x20000000: return 1
        if sec.chars&(0x80000000|0x02000000): return 2
        return 3

def parse_import_name(name,kernel_build):
    low=name.lower(); base=low
    if '@' in low:
        base,vers=low.split('@',1)
        try:
            tv,mv=vers.split('+',1); tb,tq=tv.split('.',1); mb,mq=mv.split('.',1)
            target=(2<<28)|(int(tb)<<8)|int(tq); minimum=(2<<28)|(int(mb)<<8)|int(mq)
        except Exception as e: raise PEError('invalid import version name: '+name) from e
    else:
        target=minimum=(2<<28)|(kernel_build<<8)
    if base not in KNOWN_IMPORT_IDS: raise PEError('unsupported import library '+base)
    return base,target,minimum

def build_import_blob(imports,kernel_build):
    if not imports: return b'',b'\0'*20
    parsed=[(lib,)+parse_import_name(lib.name,kernel_build) for lib in imports]
    name_table=bytearray()
    for _,base,_,_ in parsed:
        name_table+=base.encode('ascii')+b'\0'; name_table+=b'\0'*((-len(name_table))%4)
    tables=[None]*len(parsed); next_digest=b'\0'*20
    for i in range(len(parsed)-1,-1,-1):
        lib,base,tv,mv=parsed[i]; size=0x28+len(lib.addresses)*4
        t=bytearray(be32(size)+next_digest+be32(KNOWN_IMPORT_IDS[base])+be32(tv)+be32(mv)+be16(i)+be16(len(lib.addresses)))
        for a in lib.addresses: t+=be32(a)
        next_digest=hashlib.sha1(t[4:]).digest(); tables[i]=bytes(t)
    total=12+len(name_table)+sum(map(len,tables))
    return be32(total)+be32(len(name_table))+be32(len(tables))+bytes(name_table)+b''.join(tables),next_digest

def build_descriptors(pe,image):
    ps=pe.section_align; n=len(image)//ps
    desc=[[(1<<4)|pe.section_type_for_page(i*ps),b'\0'*20] for i in range(n)]
    root=b'\0'*20
    for i in range(n-1,-1,-1):
        h=hashlib.sha1(image[i*ps:(i+1)*ps]+be32(desc[i][0])+desc[i][1]).digest()
        if i: desc[i-1][1]=h
        else: root=h
    return b''.join(be32(v)+d for v,d in desc),root,n

def build_xex(pe_data,out_path,kernel_build=17559):
    pe=XboxPE(pe_data); imports=pe.imports(); image=pe.mapped(imports)
    import_blob,import_digest=build_import_blob(imports,kernel_build)
    desc_blob,section_digest,page_count=build_descriptors(pe,image)
    entries=[(XEX_HEADER_FILE_FORMAT_INFO,None),(XEX_HEADER_ENTRY_POINT,pe.image_base+pe.entry)]
    if import_blob: entries.append((XEX_HEADER_IMPORT_LIBRARIES,None))
    entries += [(XEX_HEADER_TLS_INFO,None),(XEX_HEADER_SYSTEM_FLAGS,
      XEX_SYSTEM_GAMEPAD_DISCONNECT|XEX_SYSTEM_INSECURE_SOCKETS|XEX_SYSTEM_XAM_HOOKS|XEX_SYSTEM_ALLOW_BACKGROUND_DOWNLOAD|XEX_SYSTEM_ALLOW_CONTROLLER_SWAPPING)]
    entries.sort(key=lambda x:x[0])
    sec_off=align(24+8*len(entries),8); sec_size=0x184+len(desc_blob); cur=sec_off+sec_size; offsets={}
    for key,_ in entries:
        if key==XEX_HEADER_FILE_FORMAT_INFO: cur=align(cur,8); offsets[key]=cur; cur+=16
        elif key==XEX_HEADER_TLS_INFO: cur=align(cur,8); offsets[key]=cur; cur+=16
    if import_blob: cur+=len(import_blob)
    base_off=align(cur,0x1000)
    if import_blob: offsets[XEX_HEADER_IMPORT_LIBRARIES]=base_off-len(import_blob)
    out=bytearray(base_off+len(image))
    out[:24]=XEX_MAGIC+be32(XEX_MODULE_TITLE)+be32(base_off)+be32(0)+be32(sec_off)+be32(len(entries))
    p=24
    for key,val in entries:
        if val is None: val=offsets[key]
        out[p:p+8]=be32(key)+be32(val); p+=8
    sec=bytearray(sec_size); sec[:4]=be32(sec_size); sec[4:8]=be32(len(image))
    sec[0x108:0x10c]=be32(0x174)
    sec[0x10c:0x110]=be32(XEX_IMAGE_REGION_FREE|(XEX_IMAGE_PAGE_SIZE_4KB if pe.section_align==0x1000 else 0))
    sec[0x110:0x114]=be32(pe.image_base); sec[0x114:0x128]=section_digest
    sec[0x128:0x12c]=be32(len(imports)); sec[0x12c:0x140]=import_digest
    sec[0x178:0x17c]=be32(XEX_REGION_ALL); sec[0x17c:0x180]=be32(XEX_MEDIA_ALL); sec[0x180:0x184]=be32(page_count); sec[0x184:]=desc_blob
    out[sec_off:sec_off+len(sec)]=sec
    out[offsets[XEX_HEADER_FILE_FORMAT_INFO]:offsets[XEX_HEADER_FILE_FORMAT_INFO]+16]=be32(16)+be16(0)+be16(1)+be32(len(image))+be32(0)
    out[offsets[XEX_HEADER_TLS_INFO]:offsets[XEX_HEADER_TLS_INFO]+16]=be32(0x40)+be32(0)+be32(0)+be32(0)
    if import_blob:
        io=offsets[XEX_HEADER_IMPORT_LIBRARIES]; out[io:io+len(import_blob)]=import_blob
    out[base_off:base_off+len(image)]=image
    h=hashlib.sha1(); h.update(out[sec_off+0x17c:base_off]); h.update(out[:sec_off+8])
    out[sec_off+0x164:sec_off+0x178]=h.digest()
    with open(out_path,'wb') as f: f.write(out)
    return {'base_offset':base_off,'security_offset':sec_off,'image_size':len(image),'page_count':page_count,'imports':len(imports)}

def verify(path):
    b=open(path,'rb').read()
    if len(b)<24 or b[:4]!=XEX_MAGIC: raise PEError('not XEX2')
    base=struct.unpack_from('>I',b,8)[0]; sec=struct.unpack_from('>I',b,0x10)[0]; count=struct.unpack_from('>I',b,0x14)[0]
    if base>=len(b) or sec+0x184>len(b): raise PEError('invalid offsets')
    sec_size=struct.unpack_from('>I',b,sec)[0]; image_size=struct.unpack_from('>I',b,sec+4)[0]
    pages=struct.unpack_from('>I',b,sec+0x180)[0]
    if sec+sec_size>base: raise PEError('security info overlaps basefile')
    if base+image_size>len(b): raise PEError('truncated basefile')
    return {'size':len(b),'base_offset':base,'security_offset':sec,'header_count':count,'image_size':image_size,'page_count':pages}

def main():
    ap=argparse.ArgumentParser(prog='SeriesDashXEX',description='Clean-room homebrew XEX2 packer for Xbox 360 PowerPC PE files.')
    sub=ap.add_subparsers(dest='cmd',required=True)
    p=sub.add_parser('pack'); p.add_argument('input'); p.add_argument('output'); p.add_argument('--kernel-build',type=int,default=17559)
    v=sub.add_parser('verify'); v.add_argument('xex')
    a=ap.parse_args()
    try:
        if a.cmd=='pack':
            info=build_xex(open(a.input,'rb').read(),a.output,a.kernel_build); print('XEX2 created:',a.output); print(info)
        else: print('XEX2 OK:',verify(a.xex))
        return 0
    except (OSError,PEError,struct.error) as e:
        print('ERROR:',e,file=sys.stderr); return 1
if __name__=='__main__': raise SystemExit(main())
