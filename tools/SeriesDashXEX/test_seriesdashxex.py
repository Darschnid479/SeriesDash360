import os, struct, tempfile, unittest, importlib.util

HERE=os.path.dirname(__file__)
spec=importlib.util.spec_from_file_location("sdxex",os.path.join(HERE,"seriesdashxex.py"))
sdxex=importlib.util.module_from_spec(spec); spec.loader.exec_module(sdxex)

def fixture(machine=0x1F2):
    b=bytearray(0x400); b[:2]=b'MZ'; struct.pack_into('<I',b,0x3c,0x80)
    o=0x80; b[o:o+4]=b'PE\0\0'
    struct.pack_into('<HHIIIHH',b,o+4,machine,1,0,0,0,0xE0,0x0102)
    opt=o+24; struct.pack_into('<H',b,opt,0x10B)
    struct.pack_into('<I',b,opt+0x10,0x1000); struct.pack_into('<I',b,opt+0x1c,0x82000000)
    struct.pack_into('<I',b,opt+0x20,0x1000); struct.pack_into('<I',b,opt+0x24,0x200)
    struct.pack_into('<I',b,opt+0x38,0x2000); struct.pack_into('<I',b,opt+0x3c,0x200)
    struct.pack_into('<H',b,opt+0x44,0xE); struct.pack_into('<I',b,opt+0x5c,16)
    s=opt+0xE0; b[s:s+8]=b'.text\0\0\0'
    struct.pack_into('<IIIIIIHHI',b,s+8,0x100,0x1000,0x200,0x200,0,0,0,0,0x60000020)
    for i in range(0x200): b[0x200+i]=(i*7)&255
    return bytes(b)

class Tests(unittest.TestCase):
    def test_pack_and_verify(self):
        with tempfile.TemporaryDirectory() as d:
            p=os.path.join(d,'a.xex')
            info=sdxex.build_xex(fixture(),p)
            self.assertEqual(info['page_count'],2)
            v=sdxex.verify(p)
            self.assertEqual(v['page_count'],2)
            self.assertEqual(open(p,'rb').read(4),b'XEX2')
    def test_reject_x86(self):
        with self.assertRaises(sdxex.PEError): sdxex.XboxPE(fixture(0x14c))

if __name__=='__main__': unittest.main()
